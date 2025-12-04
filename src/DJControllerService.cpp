#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    //HIT case - the track is already in the cache
    if(cache.contains(track.get_title())){
        cache.get(track.get_title());
        return 1;
    }
    //MISS case - not found
    PointerWrapper<AudioTrack> cloned_track = track.clone(); //polymorphic clone using phase 2
    if(!cloned_track){
        std:: cerr << "[ERORR] track \" " << track.get_title() << " \" cloning failed.\n";
        return 0; //MISS without eviction
    }
    cloned_track->load(); 
    cloned_track->analyze_beatgrid();
    if(cache.put(std::move(cloned_track))){
        return -1; //MISS with eviction
    }
    return 0; //MISS without eviction
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===\n";
    cache.displayStatus();
    std::cout << "====================\n";
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    // Your implementation here
    return cache.get(track_title); // Return raw pointer or nullptr if not found
}
