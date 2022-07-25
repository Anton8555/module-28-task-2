#include <iostream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;



// data

// common data for threads
string railwayStation_trainLabel;  // the symbol of the train, which stands at the station.
mutex railwayStation_access;

// condition variable: railway station
//bool wait;
mutex wait_access;

// condition variable: message output queue to the console
//bool fOut;
mutex fOut_access;






/*!
 * @brief Subroutine for threads.
 * @param label - symbol of the train.
 * @param time - train travel time.
 */
void thread_train(const string& label, int time) {
    // train simulation on the way
    this_thread::sleep_for(chrono::seconds(time));

    fOut_access.lock();
    cout << "Train '" << label << "' is waiting for a free seat at the station.\n";
    fOut_access.unlock();

    // if blocked, then wait
    wait_access.lock();
    wait_access.unlock();

    // celebrating our arriving train
    railwayStation_access.lock();
    railwayStation_trainLabel = label;
    railwayStation_access.unlock();

    // and display a message about it
    fOut_access.lock();
    cout << "Train '" << label << "' arrived at the station.\n";
    fOut_access.unlock();
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
    cout << endl;

    // start threads
    railwayStation_trainLabel = "none";
    wait_access.lock();  // blocking the station
    for(auto& t: train)
        t.track = new thread(thread_train, t.label, t.time);

    // station work
    while(true) {
        // data
        bool flag;
        string command;
        string trainLabel;

        // momentarily unlock the wait mutex
        wait_access.unlock();
        wait_access.lock();

        // read the designation of the arrived train
        railwayStation_access.lock();
        trainLabel = railwayStation_trainLabel;
        railwayStation_access.unlock();

        // work with station data
        // if a train arrives at the station
        while( trainLabel != "none" ) {
            // command input
            fOut_access.lock();
            cout << "To send train '" << trainLabel << "', enter the command 'depart':";
            cin >> command;
            fOut_access.unlock();

            // team analysis
            if( command == "depart" ) {
                fOut_access.lock();
                cout << "Train '" << trainLabel << "' has departed.\n";
                fOut_access.unlock();

                // departed train mark
                for(auto &t: train)
                    if (trainLabel == t.label) {
                        t.flag = true;
                        break;
                    }

                // note that there is no train at the station
                railwayStation_access.lock();
                railwayStation_trainLabel = trainLabel = "none";
                railwayStation_access.unlock();
            }
        }

        // if all trains have been at the station, then exit the station maintenance cycle
        flag = true;
        for(auto &t: train)
            flag &= t.flag;
        if( flag )
            break;

        // pause
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // unblocking the station
    wait_access.unlock();

    return 0;
}
