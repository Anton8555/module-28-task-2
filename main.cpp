#include <iostream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;

// common data for threads
string railwayStation_trainLabel;  // the symbol of the train, which stands at the station.
mutex railwayStation_access;


/*!
 * @brief Subroutine for threads.
 * @param label - symbol of the train.
 * @param time - train travel time.
 */
void thread_train(const string& label, int time) {
    // data
    bool fFirst;

    // train simulation on the way
    this_thread::sleep_for(chrono::seconds(time));

    // station occupancy check cycle
    fFirst = false;
    while( true ) {
        // work with station data
        railwayStation_access.lock();
        // if the station is free
        if( railwayStation_trainLabel == "none" ) {
            // then display the corresponding message
            cout << "Train '" << label << "' arrived at the station.\n";
            railwayStation_trainLabel = label;  // the mark at the station of the arriving train
            railwayStation_access.unlock();
            break;
        } else {
            // otherwise, display a message about waiting for a train of a free place at the station once
            if( !fFirst ) {
                fFirst = true;
                cout << "Train '" << label << "' is waiting for a free seat at the station.\n";
            }
        }
        railwayStation_access.unlock();

        // wait until the already occupied space becomes free.
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}



int main() {
    // data
    const int N = 3;
    struct Train {
        string label;             // train symbol
        int time = 0;             // train travel time
        thread* track = nullptr;  // thread object
        bool flag = false;        // train arrival flag
    } train[N];

    // data input
    for(auto &t: train) {
        cout << "Enter label:";
        cin >> t.label;
        cout << "Enter time:";
        cin >> t.time;
    }

    // start threads
    railwayStation_trainLabel = "none";
    for(auto& t: train) {
        t.track = new thread(thread_train, t.label, t.time);
    }

    // station work
    while(true) {
        // data
        bool flag;
        string command;

        // work with station data
        railwayStation_access.lock();
        // if a train arrives at the station
        if( railwayStation_trainLabel != "none" ) {
            // command input
            cout << "To send train '" << railwayStation_trainLabel << "', enter the command 'depart':";
            cin >> command;

            // team analysis
            if( command == "depart" ) {
                cout << "Train '" << railwayStation_trainLabel << "' has departed.\n";

                // departed train mark
                for(auto &t: train)
                    if( railwayStation_trainLabel == t.label )
                        t.flag = true;
                railwayStation_trainLabel = "none";
            }
        }
        railwayStation_access.unlock();

        // if all trains have been at the station, then exit the station maintenance cycle
        flag = true;
        for(auto &t: train)
            flag &= t.flag;
        if( flag )
            break;

        // wait
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // shutting down threads
    for( auto &t: train )
        if( t.track->joinable() )
            t.track->join();

    return 0;
}
