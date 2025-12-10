#include "Playlist.h"
#include "AudioTrack.h"
#include <iostream>
#include <algorithm>
Playlist::Playlist(const std::string& name) 
    : head(nullptr), playlist_name(name), track_count(0) {
    std::cout << "Created playlist: " << name << std::endl;
}
// TODO: Fix memory leaks!
// Students must fix this in Phase 1

//we chose to own the AudioTrack pointers in the playlist.

Playlist::~Playlist() {
    #ifdef DEBUG
    std::cout << "Destroying playlist: " << playlist_name << std::endl;
    #endif
    while (head != nullptr){
        PlaylistNode* temp = head->next;
        delete head->track; // delete the AudioTrack - its a clone so we own it. 
        delete head;
        head = temp;
    }
    track_count = 0;
}

Playlist::Playlist(const Playlist& other):
head(nullptr), playlist_name(other.playlist_name), track_count(other.track_count)
{
     #ifdef DEBUG
    std::cout << "Playlist copy constructor called for: " << other.playlist_name << std::endl;
    #endif
    if(other.head == nullptr){
        return;
    }
    this->head = new PlaylistNode(other.head->track);
    PlaylistNode* currother = other.head->next;
    PlaylistNode* currthis = this->head;
    while(currother){
        currthis->next = new PlaylistNode(currother->track);
        currthis = currthis->next;
        currother = currother->next;
    }


}

Playlist& Playlist::operator=(const Playlist& other) {
    if(this == &other){
        return *this;
    }
    PlaylistNode* curr = this->head;
    while(curr){
        PlaylistNode* temp = curr->next;
        delete curr;
        curr = temp;
    }
    this->playlist_name = other.playlist_name;
    this->track_count = other.track_count;
    this->head = nullptr;
    if(other.head == nullptr){
        return *this;
    }
    //if other head isnt null than we will coppy all the list:
    this->head = new PlaylistNode(other.head->track->clone().release()); // deep copy using clone
    PlaylistNode* currother = other.head->next;
    PlaylistNode* currthis = this->head;
    while(currother){
        currthis->next = new PlaylistNode(currother->track->clone().release()); // deep copy using clone
        currthis = currthis->next;
        currother = currother->next;
    }
    return *this;

}

void Playlist::add_track(AudioTrack* track) {
    if (!track) {
        std::cout << "[Error] Cannot add null track to playlist" << std::endl;
        return;
    }
    //use the track inserted to the fuction , its a clone so we own it
    PlaylistNode* new_node = new PlaylistNode(track);
    new_node->next = head;
    head = new_node;
    // Increment track count
    track_count++;
    std::cout << "Added '" << track->get_title() << "' to playlist '" 
              << playlist_name << "'" << std::endl;
}

void Playlist::remove_track(const std::string& title) {
    PlaylistNode* current = head;
    PlaylistNode* prev = nullptr;

    // Find the track to remove
    while (current && current->track->get_title() != title) {
        prev = current;
        current = current->next;
    }

    if (current) {
        // Remove from the playlist
        if (prev) {
            prev->next = current->next;
        } else {
            head = current->next;
        }
        delete current->track; // Free the AudioTrack memory, since we own it
        delete current;  // Free the node memory, since we own it
        track_count--;
        std::cout << "Removed '" << title << "' from playlist" << std::endl;

    } else {
        std::cout << "Track '" << title << "' not found in playlist" << std::endl;
    }
}

void Playlist::display() const {
    std::cout << "\n=== Playlist: " << playlist_name << " ===" << std::endl;
    std::cout << "Track count: " << track_count << std::endl;

    PlaylistNode* current = head;
    int index = 1;

    while (current) {
        std::vector<std::string> artists = current->track->get_artists();
        std::string artist_list;

        std::for_each(artists.begin(), artists.end(), [&](const std::string& artist) {
            if (!artist_list.empty()) {
                artist_list += ", ";
            }
            artist_list += artist;
        });

        AudioTrack* track = current->track;
        std::cout << index << ". " << track->get_title() 
                  << " by " << artist_list
                  << " (" << track->get_duration() << "s, " 
                  << track->get_bpm() << " BPM)" << std::endl;
        current = current->next;
        index++;
    }

    if (track_count == 0) {
        std::cout << "(Empty playlist)" << std::endl;
    }
    std::cout << "========================\n" << std::endl;
}

AudioTrack* Playlist::find_track(const std::string& title) const {
    PlaylistNode* current = head;

    while (current) {
        if (current->track->get_title() == title) {
            return current->track;
        }
        current = current->next;
    }

    return nullptr;
}

int Playlist::get_total_duration() const {
    int total = 0;
    PlaylistNode* current = head;

    while (current) {
        total += current->track->get_duration();
        current = current->next;
    }

    return total;
}

std::vector<AudioTrack*> Playlist::getTracks() const {
    std::vector<AudioTrack*> tracks;
    PlaylistNode* current = head;
    while (current) {
        if (current->track)
            tracks.push_back(current->track);
        current = current->next;
    }
    return tracks;
}