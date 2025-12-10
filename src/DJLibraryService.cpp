#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist), library() {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */

//distructor implmentation
DJLibraryService::~DJLibraryService() {
    for(AudioTrack* track : library){
        if(track != nullptr){
            delete track;
        }
    }
    library.clear(); // Clear the library vector
}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */

void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) 
{
        //Todo: Implement buildLibrary method 

    library.clear();// Clear existing library before building new one
    //for each track info, create appropriate AudioTrack and add to library
    for(size_t i = 0 ; i< library_tracks.size(); ++i) {
        AudioTrack* track = nullptr; // Initialize to nullptr
        // Create track based on type
        if(library_tracks[i].type == "MP3"){
            track = new MP3Track(library_tracks[i].title, library_tracks[i].artists, library_tracks[i].duration_seconds, library_tracks[i].bpm , library_tracks[i].
            extra_param1, library_tracks[i].extra_param2);
        }
        else if(library_tracks[i].type=="WAV"){
            track = new WAVTrack(library_tracks[i].title, library_tracks[i].artists, library_tracks[i].duration_seconds, library_tracks[i].bpm , library_tracks[i].
            extra_param1, library_tracks[i].extra_param2);
        }
        // Add track to library if created successfully
        if(track!=nullptr){
            library.push_back(track);
        }
    }
    // Log library build completion
    std::cout << "[INFO] Track library built: " << library_tracks.size() << " tracks loaded" << std::endl;
}

/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    // Your implementation here
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    // Your implementation here
    AudioTrack* track = this->playlist.find_track(track_title); // Use Playlist's find_track method
    if (!track){
        return nullptr;
    }
    return track;
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                               const std::vector<int>& track_indices) {
    // Your implementation here

    std::cout << " [INFO] Loading playlist: " << playlist_name << std::endl;
    // Reset current playlist with new name
    this->playlist = Playlist(playlist_name); 
    int count = 0;
    // Iterate over provided track indices                                            
    for (size_t i = 0; i < track_indices.size(); i++) {
        // Validate index range
        if (track_indices[i] < 1 || track_indices[i] > (int)library.size()) {
            std::cout << " [WARNING] Invalid track index: " << track_indices[i] << std::endl;
            continue;
        }
        // Retrieve track from library
        AudioTrack* track = library[track_indices[i] - 1]; 
        // Clone the track to avoid shared ownership
        PointerWrapper<AudioTrack> t = track->clone();// Polymorphic clone uses phase 2
        AudioTrack* raw = t.release();// Get raw pointer from PointerWrapper
        // Check cloning success
        if (!raw) {
            std::cerr << "[ERROR] Track: " << track->get_title() << " failed to clone." << std::endl;
            continue;
        }
        // Load and analyze the track before adding to playlist
        raw->load();
        raw->analyze_beatgrid();
        // Add to playlist and increment count
        this->playlist.add_track(raw);
        count++;
    }
    // Final log of playlist loading
    std::cout << " [INFO] Playlist loaded: " 
              << playlist_name << " (" << count << " tracks)" << std::endl;
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    // Your implementation here
    std::vector<std::string> titles; 
    std::vector<AudioTrack*> tracks = playlist.getTracks(); // using Playlist's getTracks method - does not transfer ownership
    for(size_t i = 0 ; i < tracks.size(); ++i){
        if(tracks[i]!= nullptr){
            titles.push_back(tracks[i]-> get_title()); // Extract title and add to vector titles
        }
    }
    return titles;
}
