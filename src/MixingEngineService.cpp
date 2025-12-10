#include "MixingEngineService.h"
#include <iostream>
#include <memory>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : decks(), active_deck(1), auto_sync(false), bpm_tolerance(0)
{
    // Your implementation here
    decks[0] = nullptr;
    decks[1] = nullptr;
    std::cout << "[MixingEngineService] Initialized with 2 empty decks.\n";
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    // Your implementation here
        std::cout << "[MixingEngineService] Cleaning up decks...\n";
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
    std::cout << "\n=== Loading Track to Deck ===\n";
    
    //---- case 1 - both decks empty ----
    if(decks[0]==nullptr && decks[1]==nullptr){
        // Clone the track
        PointerWrapper<AudioTrack> cloned_track = track.clone();
        if(!cloned_track){
            std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
            return -1;
        }
        std::cout << "[Deck Switch] Target deck: 0\n";
        cloned_track->load();
        cloned_track->analyze_beatgrid();
        if (auto_sync) { //if auto-sync enabled , send a log message that sync cannot be done
            std::cout<<"[Sync BPM] Cannot sync - one of the decks is empty.\n";
        }
        // Load into deck 0
        decks[0] = cloned_track.release();
        // Set active deck to 0
        active_deck = 0;
        // Log completion
        std::cout << "[Load Complete] '" << track.get_title() << "' is now loaded on deck 0\n";
        std::cout << "[Active Deck] Switched to deck " << active_deck << "\n";
        return active_deck;
    }
    //---- case 2 - at least one deck occupied ----
    size_t target = 1-active_deck;
    std::cout << "[Deck Switch] Target deck: " << target << "\n";
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
    // Auto-sync BPM if enabled and tracks not mixable, in case one is empty log message
   if (auto_sync && !can_mix_tracks(cloned_track)) {
        sync_bpm(cloned_track);
    }
    // Load the cloned track into the target deck
    decks[target] = cloned_track.release();
    std::cout << "[Load Complete] '" << track.get_title() << "' is now loaded on deck " << target << "\n";
    // Switch active deck
    active_deck = target;
    std::cout << "[Active Deck] Switched to deck " << active_deck << "\n";
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
    // Check if active deck is occupied
    if(decks[active_deck]==nullptr || !track){
        return false;
    }
    bool canMix = false; 
    if(std::abs(decks[active_deck]->get_bpm() - track->get_bpm()) <= bpm_tolerance){ //compare BPMs
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

    //one of the decks is empty
    if(decks[active_deck]==nullptr || !track){
        std::cout<<"[Sync BPM] Cannot sync - one of the decks is empty.\n";
        return;
    }
    int og_bpm = track->get_bpm(); 
    int act_bpm =decks[active_deck]->get_bpm();
    int avg_bpm = (og_bpm + act_bpm) / 2;
    // Set the track's BPM to the average
    track->set_bpm(avg_bpm); //set_bpm method implemeted in audioTrack.h
    std::cout << "[Sync BPM] Syncing BPM from " << og_bpm << " to " << avg_bpm << "\n";
}
