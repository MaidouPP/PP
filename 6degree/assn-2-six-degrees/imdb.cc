using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <cstring>
#include <cstdlib>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

struct kStruct {
  const void* key;
  const void* array;
};

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
  int numActor = *(int*)actorFile;
  kStruct p = {&player, actorFile};
  void* foundPlayer = bsearch(&p, actorFile, numActor, sizeof(int), compareActor); 
  if(!foundPlayer) return false;
  else {
    extractFilms(films, foundPlayer, player);
    return true;
  }
}

void imdb::extractFilms(vector<film>& films, void* found, const string& player) const {
  char* playerAddr = *(int*)found + (char*)actorFile;
  char* tmp = "";
  int year;
  int offset = (player.size()+1)%2 == 0 ? player.size()+1 : player.size()+2;
  int numberOfFilms = *(short*)(playerAddr + offset);
  offset = (offset+sizeof(short)) % 4 == 0 ? offset + sizeof(short) : offset + sizeof(short) + 4 - (offset+sizeof(short)) % 4;
  for(int i=0; i<numberOfFilms; i++) {
    int offsetOfFilm = *(int*)(playerAddr + offset);
    tmp = (char*)(movieFile + offsetOfFilm);
    string nameOfFilm = string(tmp);
    year = *(unsigned char*)(tmp + nameOfFilm.size() + 1);
    offset += sizeof(int);
    film thisFilm = {string(tmp), year + 1900};
    films.push_back(thisFilm);
  }
}

int compareActor(const void* key, const void* candidate) {
  kStruct p = *(kStruct*)key;
  char* elemString = *((char**)key + 1) + *(int*)candidate;
  return strcmp(*(char**)(p.key), elemString);
}

bool imdb::getCast(const film& movie, vector<string>& players) const {
  int numFilm = *(int*)movieFile;
  kStruct p = {&movie, movieFile};
  void* foundMovie = bsearch(&p, movieFile, numFilm, sizeof(int), compareMovie);
  if (!foundMovie) return false;
  else {
    extractPlayers(players, foundMovie, movie);
    return true;
  }
}

void imdb::extractPlayers(vector<string>& players, void* found, const film& film) const {
  char* filmAddr = *(int*)found + (char*)movieFile;
  string nameOfFilm = film.title;
  int offset = nameOfFilm.size() + 1 + sizeof(unsigned char);
  offset = offset % 2 == 0 ? offset : offset + 1 ;
  int numberOfPlayers = *(short*)(filmAddr + offset);
  offset += sizeof(short);
  offset = offset % 4 == 0 ? offset : offset + 2;
  for(int i=0; i<numberOfPlayers; i++) {
    int offsetOfPlayer = *(int*)(filmAddr + offset);
    string player = (char*)(actorFile + offsetOfPlayer);
    offset += sizeof(int);
    players.push_back(player);
  }
}

int compareMovie(const void* key, const void* candidate) {
  kStruct p = *(kStruct*)key;
  film keyFilm = *(film*)(p.key);
  char* elemAddr = (char*)(p.array) + *(int*)candidate;
  film elemFilm = {string(elemAddr), 1900 + *(unsigned char*)(elemAddr + string(elemAddr).size() + 1)};
  if (keyFilm == elemFilm) return 0;
  else if (keyFilm < elemFilm) return -1;
  else return 1;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
