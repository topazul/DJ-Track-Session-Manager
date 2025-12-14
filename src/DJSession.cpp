
#include "DJSession.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>

// ========== CONSTRUCTORS & RULE OF 5 ==========


DJSession::DJSession(const std::string& name, bool play_all)
    : session_name(name),
    library_service(),
    controller_service(),
    mixing_service(),
    config_manager(),
    session_config(),
    track_titles(),
    play_all(play_all),
    stats()
      {
    std::cout << "DJ Session System initialized: " << session_name << std::endl;
}

// Destructor will call the destructors of member objects automatically
DJSession::~DJSession() { 
    std::cout << "Shutting down DJ Session System: " << session_name << std::endl;
}

// ========== CORE FUNCTIONALITY ==========
bool DJSession::load_playlist(const std::string& playlist_name)  {
    std::cout << "[System] Loading playlist: " << playlist_name << "\n";
    
    // Find the playlist in the session config
    auto it = session_config.playlists.find(playlist_name);
    if (it == session_config.playlists.end()) {
        std::cerr << "[ERROR] Playlist '" << playlist_name << "' not found in configuration.\n";
        return false;
    }
    
    // Load playlist from track indices
    library_service.loadPlaylistFromIndices(playlist_name, it->second);
    
    if (library_service.getPlaylist().is_empty()) {
        return false;
    }
    
    track_titles = library_service.getTrackTitles();
    return true;
}

/**
 * TODO: Implement load_track_to_controller method
 * 
 * REQUIREMENTS:
 * 1. Track Retrieval
 *    - Find track in library using track name
 *    - Handle case when track is not found
 *    - Update error stats if track not found
 * 
 * 2. Controller Loading
 *    - Delegate loading to controller_service
 *    - Pass track by reference to controller
 * 
 * 3. Return Values
 *    1: Cache HIT
 *    0: Cache MISS (or error)
 *   -1: Cache MISS with eviction
 * 
 * @param track_name: Name of track to load
 * @return: Cache operation result code

 */
int DJSession::load_track_to_controller(const std::string& track_name) {
    // Your implementation here
    AudioTrack* track = library_service.findTrack(track_name); // find track in library
    // track not found
    if (track == nullptr) {
        std::cerr << "[ERROR] Track '" << track_name << "' not found in library." << std::endl;
        stats.errors++;
        return 0; // MISS due to not found
    }
    std::cout << "[System] Loading track '" << track_name << "' to controller..." << std::endl;
    int res = controller_service.loadTrackToCache(*track); // load track to cache, the func will clone it.
    // HIT
    if(res == 1){ 
        controller_service.displayCacheStatus(); 
        stats.cache_hits++;
    }
    // MISS
    else if(res == 0){ 
        controller_service.displayCacheStatus(); 
        stats.cache_misses++;
    }
    // MISS with eviction
     else if(res == -1){
        controller_service.displayCacheStatus();
        stats.cache_misses++;
        stats.cache_evictions++;
    }
    return res;
}
/**
 * TODO: Implement load_track_to_mixer_deck method
 * 
 * @param track_title: Title of track to load to mixer
 * @return: Whether track was successfully loaded to a deck
 */
bool DJSession::load_track_to_mixer_deck(const std::string& track_title) {
    std::cout << "[System] Delegating track transfer to MixingEngineService for: " << track_title << std::endl;
    // your implementation here
    AudioTrack* track = controller_service.getTrackFromCache(track_title); // get track from cache
    // track not found in cache
    if (track == nullptr) {
        std::cerr << "[ERROR] Track: \"" << track_title << "\" not found in cache" << std::endl;
        stats.errors++;
        return false;
    }
    // load track to mixer deck
    int res = mixing_service.loadTrackToDeck(*track); //load track to deck, the func will clone it.
    if(res == -1){ // failed to load to deck
        stats.errors++;
        std::cerr << "[ERROR] Failed to load track '" << track_title << "' to mixer deck." << std::endl;
        return false;
    }
    else if(res == 0 ){ // loaded to deck 0
        stats.deck_loads_a++;
        stats.transitions++;
    }
    else{ // loaded to deck 1
        stats.deck_loads_b++;
        stats.transitions++;
    }
    mixing_service.displayDeckStatus(); // display deck status
    return true; // successfully loaded to deck 0 or deck 1
}

/**
 * @brief Main simulation loop that orchestrates the DJ performance session.
 * @note Updates session statistics (stats) throughout processing
 * @note Calls print_session_summary() to display results after playlist completion
 */
void DJSession::simulate_dj_performance() {
    std::cout << "=== DJ Controller System ===" << std::endl;
    std::cout << "Starting interactive DJ session..." << std::endl;
    // 1. Load configuration
    if (!load_configuration()) {
        std::cerr << "[ERROR] Failed to load configuration. Aborting session." << std::endl;
        return;
    }
    
    // 2. Build track library from config
    library_service.buildLibrary(session_config.library_tracks);
    
    // 3. Get available playlists from config
    if (session_config.playlists.empty()) {
        std::cerr << "[ERROR] No playlists found in configuration. Aborting session." << std::endl;
        return;
    }
    std::cout << "\nStarting DJ performance simulation..." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Capacity: " << session_config.controller_cache_size << " slots (LRU policy)" << std::endl;
    std::cout << "\n--- Processing Tracks ---" << std::endl;

    // Your implementation here
    std::vector<std::string> playlist_to_play;
    bool session_active = true;
    //main loop
    while (session_active) {
        playlist_to_play.clear();
        // if auto mode -A, add all playlists to vector
        if (play_all) {
            for (const auto& pair : session_config.playlists) {
                playlist_to_play.push_back(pair.first);
            }
            //sort playlist names alphabetically
            std::sort(playlist_to_play.begin(), playlist_to_play.end());

            //stops the main loop after playing all playlists
            if (playlist_to_play.empty()) {
                break;
            }

        } 
        // else interactive mode -I, display menu to select playlist
        else {
            std::string selection = display_playlist_menu_from_config();
            if (selection.empty()) {
                break;
            }      
            playlist_to_play.push_back(selection);
        }
        //for each playlist in the vector, load and process tracks
        for (const std::string& playlist_name : playlist_to_play) {
            if (!load_playlist(playlist_name)) {
                std::cerr << "[ERROR] Failed to load playlist: " << playlist_name << std::endl;
                continue;
            }
            //reverse track titles to process in order
            std::reverse(track_titles.begin(), track_titles.end());
            //for each track in the playlist, load to controller and then to mixer deck
            for (const std::string& track_name : track_titles) {
                std::cout << "\n--- Processing: " << track_name << " ---" << std::endl;
                stats.tracks_processed++;
                
                load_track_to_controller(track_name);
                
                if (!load_track_to_mixer_deck(track_name)) {
                    std::cerr << "[ERROR] Failed to load track to mixer deck: " << track_name << std::endl;
                    continue;
                }   
            } 
            print_session_summary(); // print summary after each playlist
        }
        //if in auto mode -A, end session after playing all playlists
        if (play_all) {
            session_active = false;
        }
    }
    //end of main loop
    std::cout << "Session cancelled by user or all playlists played." << std::endl;

}


/* 
 * Helper method to load session configuration from file
 * 
 * @return: true if configuration loaded successfully; false on error
 */
bool DJSession::load_configuration() {
    const std::string config_path = "bin/dj_config.txt";
    
    std::cout << "Loading configuration from: " << config_path << std::endl;
    
    if (!SessionFileParser::parse_config_file(config_path, session_config)) {
        std::cerr << "[ERROR] Failed to parse configuration file: " << config_path << std::endl;
        return false;
    }
    
    std::cout << "Configuration loaded successfully." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Size: " << session_config.controller_cache_size << " slots" << std::endl;
    mixing_service.set_auto_sync(session_config.auto_sync);
    mixing_service.set_bpm_tolerance(session_config.bpm_tolerance);
    //update cache size in LRUCache
    controller_service.set_cache_size(session_config.controller_cache_size);
    return true;
}

std::string DJSession::display_playlist_menu_from_config() {
    if (session_config.playlists.empty()) {
        return "";
    }
    
    std::cout << "\n=== Available Playlists ===" << std::endl;
    
    // Build sorted list of playlist names
    std::vector<std::string> playlist_names;
    for (const auto& pair : session_config.playlists) {
        playlist_names.push_back(pair.first);
    }
    std::sort(playlist_names.begin(), playlist_names.end());
    
    // Display numbered list
    for (size_t i = 0; i < playlist_names.size(); ++i) {
        std::cout << (i + 1) << ". " << playlist_names[i] << std::endl;
    }
    std::cout << "0. Cancel" << std::endl;
    
    // Prompt for user selection with validation
    int selection = -1;
    while (true) {
        std::cout << "\nSelect a playlist (1-" << playlist_names.size() << ", 0 to cancel): ";
        std::string input;
        
        if (!std::getline(std::cin, input)) {
            std::cout << "\n[ERROR] Input error. Cancelling session." << std::endl;
            return "";
        }
        
        std::stringstream ss(input);
        if (ss >> selection && ss.eof()) {
            if (selection == 0) {
                return "";
            } else if (selection >= 1 && selection <= static_cast<int>(playlist_names.size())) {
                std::string selected_name = playlist_names[selection - 1];
                std::cout << "Selected: " << selected_name << std::endl;
                return selected_name;
            }
        }
        
        std::cout << "Invalid selection. Please enter a number between 1 and " 
                  << playlist_names.size() << ", or 0 to cancel." << std::endl;
    }
}

void DJSession::print_session_summary() const {
    std::cout << "\n=== DJ Session Summary ===" << std::endl;
    std::cout << "Session: " << session_name << std::endl;
    std::cout << "Tracks processed: " << stats.tracks_processed << std::endl;
    std::cout << "Cache hits: " << stats.cache_hits << std::endl;
    std::cout << "Cache misses: " << stats.cache_misses << std::endl;
    std::cout << "Cache evictions: " << stats.cache_evictions << std::endl;
    std::cout << "Deck A loads: " << stats.deck_loads_a << std::endl;
    std::cout << "Deck B loads: " << stats.deck_loads_b << std::endl;
    std::cout << "Transitions: " << stats.transitions << std::endl;
    std::cout << "Errors: " << stats.errors << std::endl;
    std::cout << "=== Session Complete ===" << std::endl;
}