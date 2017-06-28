#include "path.h"
#include <iostream>
using namespace std;

/**
 * All paths are designed to store the path from a specified
 * actor or actress to another actor or actress through a series
 * of movie-player connections.  A new path is always a partial
 * path because it only knows of the first player in the chain.
 * As a result, the embedded vector should be set to be empty, because
 * each entry in the vector is one leg in the path from an actor to
 * another.
 */

path::path(const string& player) : startPlayer(player) {} 
// ommission of links from init list calls the default constructor

/**
 * Simply tack on a new connection pair to the end of the links vector.
 * It ain't our business to be checking for consistency of connection, as
 * that's the resposibility of the surrounding class to decide (or at
 * least we're making it their business.
 */

void path::addConnection(const film& movie, const string& player)
{
  links.push_back(connection(movie, player));
} 

/**
 * Remove the last connection pair 
 * if there is one.
 */

void path::undoConnection()
{
  if (links.size() == 0) return;
  links.pop_back();
}

/**
 * Returns the last player (actor/actress) currently 
 * in the path.
 */

const string& path::getLastPlayer() const
{
  if (links.size() == 0) return startPlayer;
  return links.back().player;
}

bool path::generateShortestPath(imdb& db, string& target, string& source) {
  path startPath(source);
  list<path> partialPaths;
  partialPaths.push_back(startPath);
  set<string> previouslySeenActors;
  set<film> previouslySeenFilms;
  previouslySeenActors.insert(source);
  
  while(partialPaths.size() && partialPaths.front().getLength() <= 5) {
    path startPath = partialPaths.front();
    partialPaths.pop_front();
    if (generateNewPath(db, partialPaths, startPath, previouslySeenFilms, previouslySeenActors, target)) {
      return true;
    }
  }
  return false;
}

bool path::generateNewPath(imdb& db, list<path>& partialPaths, path& currentPath, set<film>& previouslySeenFilms, set<string>& previouslySeenActors, string& target) {
  vector<film> films;
  string lastPlayer = currentPath.getLastPlayer();
  db.getCredits(lastPlayer, films);
  
  for(int i=0; i<films.size(); i++) {
    if (previouslySeenFilms.find(films[i]) == previouslySeenFilms.end()) {
      previouslySeenFilms.insert(films[i]);
    } else continue;
    vector<string> players;
    db.getCast(films[i], players);
    
    for(int j=0; j<players.size(); j++) {
      if (previouslySeenActors.find(players[j]) == previouslySeenActors.end()) {
	previouslySeenActors.insert(players[j]);
      } else continue;
      path nextPath = currentPath;
      nextPath.links.push_back(connection(films[i], players[j]));
      if (players[j] == target) {
	cout << nextPath;
	return true;
      }
      partialPaths.push_back(nextPath);
    }
  }
  return false;
}

void path::reverse()
{
  // construct the reverse
  path reverseOfPath(getLastPlayer());
  for (int i = links.size() - 1; i > 0; i--)
    reverseOfPath.addConnection(links[i].movie, links[i-1].player);
  if (links.size() > 0)
    reverseOfPath.addConnection(links[0].movie, startPlayer);

  // then assign self to its reverse
  *this = reverseOfPath;
}

ostream& operator<<(ostream& os, const path& p)
{
  if (p.links.size() == 0) return os << string("[Empty path]") << endl;
  
  os << "\t" << p.startPlayer << " was in ";
  for (int i = 0; i < (int) p.links.size(); i++) {
    os << "\"" << p.links[i].movie.title << "\" (" << p.links[i].movie.year << ") with " 
       << p.links[i].player << "." << endl;
    if (i + 1 == (int) p.links.size()) break;
    os << "\t" << p.links[i].player << " was in ";
  }

  return os;
}
