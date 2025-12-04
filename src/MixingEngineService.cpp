#include "MixingEngineService.h"
#include <iostream>
#include <memory>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : active_deck(0), auto_sync(false), bpm_tolerance(0)
{
    // Your implementation here
    decks[0] = nullptr;
    decks[1] = nullptr;
    std::cerr << "[MixingEngineService] Initialized with 2 empty decks.\n";
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    // Your implementation here
        std::cerr << "[MixingEngineService] Cleaning up decks...\n";
        if(decks[0]!=nullptr){
            delete decks[0];
            decks[0]=nullptr;
        }
         if(decks[1]!=nullptr){
            delete decks[1];
            decks[1]=nullptr;
        }

}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    // Your implementation here
    //case 1 - both decks empty
    if(decks[0]==nullptr && decks[1]==nullptr){
        std::cerr << "\n=== Loading Track to Deck ===\n";
        // Clone the track
        PointerWrapper<AudioTrack> cloned_track = track.clone();
        if(!cloned_track){
            std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
            return -1;
        }
        cloned_track->load();
        cloned_track->analyze_beatgrid();
        // Load into deck 0
        decks[0] = cloned_track.release();
        // Set active deck to 0
        active_deck = 0;
        return active_deck;
    }
    //case 2 - at least one deck occupied
    size_t target = 1-active_deck;
    std::cerr << "[Deck Switch] Target deck: " << target << "\n";
    // Unload target deck if occupied
    if(decks[target]!= nullptr){
       delete decks[target];
       decks[target] = nullptr;
    }
    // Clone the track
    PointerWrapper<AudioTrack> cloned_track = track.clone();
    if(!cloned_track){
        std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
        return -1;
    }    
    cloned_track->load();
    cloned_track->analyze_beatgrid();
    // Auto-sync BPM if enabled
    if(decks[active_deck]!=nullptr && auto_sync && !can_mix_tracks(cloned_track)){
        std::cerr << "[BPM Sync] Synchronizing BPM for '"<< track.get_title() << "'\n";
        sync_bpm(cloned_track);
    }
    // Load the cloned track into the target deck
    decks[target] = cloned_track.release();
    std::cerr << "[Load Complete] '" << track.get_title() << "' is now loaded on deck " << target << "\n";
    // Unload previous deck
    if(decks[active_deck]!=nullptr){
        std::cerr << "[Unload] Unloading previous deck " << active_deck << " (" << decks[active_deck]->get_title() << ")\n";
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }
    // Switch active deck
    active_deck = target;
    std::cerr << "[Active Deck] Switched to deck " << active_deck << "\n";
    return active_deck;
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    // Your implementation here
    if(decks[active_deck]==nullptr || !track){
        return false;
    }
    bool canMix = false;
    if(std::abs(decks[active_deck]->get_bpm() - track->get_bpm()) <= bpm_tolerance){
        canMix = true;
    }
    return canMix;
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    // Your implementation here
    if(decks[active_deck]==nullptr || !track){
        return;
    }
    int og_bpm = track->get_bpm(); 
    int act_bpm =decks[active_deck]->get_bpm();
    int avg_bpm = (og_bpm + act_bpm) / 2;
    // Set the track's BPM to the average
    track->set_bpm(avg_bpm);
    std::cerr << "[Sync BPM] Syncing BPM from " << og_bpm << " to " << avg_bpm << "\n";
}
