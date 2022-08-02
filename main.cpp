#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
using namespace std;



// data

// common data for threads
queue<string> railwayStation;
mutex railwayStation_access;

// condition variable: message output queue to the console
mutex fOut_access;






/*!
 * @brief Subroutine for threads.
 * @param label - symbol of the train.
 * @param time - train travel time.
 */
void thread_train(const string& label, int time) {
    // train simulation on the way
    this_thread::sleep_for(chrono::seconds(time));

    // message output
    fOut_access.lock();
    cout << "Train '" << label << "' is waiting for a free seat at the station.\n";
    fOut_access.unlock();

    // queuing a train
    railwayStation_access.lock();
    railwayStation.push(label);
    railwayStation_access.unlock();
}



int main() {
    // data

    // train data
    const int N = 3;
    struct Train {
        string label;             // train symbol
        int time = 0;             // train travel time
        thread* track = nullptr;  // thread object
        bool flag = false;        // train arrival flag
    } train[N];
    // flag
    bool everyoneWasAtTheStation;

    // data input
    for(auto &t: train) {
        cout << "Enter label:";
        cin >> t.label;
        cout << "Enter time:";
        cin >> t.time;
    }
    cout << endl;

    // start threads
    for(auto& t: train)
        t.track = new thread(thread_train, t.label, t.time);

    // station work
    do{
        // data
        string trainLabel;

        // pause (poll the queue twice per second)
        this_thread::sleep_for(chrono::milliseconds(500));

        // get value from queue
        railwayStation_access.lock();
        if( !railwayStation.empty() ) {
            trainLabel = railwayStation.front();
            railwayStation.pop();
        } else {
            trainLabel = "none";
        }
        railwayStation_access.unlock();

        // if it is a train designation
        if( trainLabel != "none" ) {
            // data
            string command;
            bool commandCompletedFlag;

            // then the conclusion about the arrival of the train at the station
            fOut_access.lock();
            cout << "Train '" << trainLabel << "' arrived at the station.\n";
            fOut_access.unlock();

            // command input and execution cycle
            commandCompletedFlag = false;  // there is no executed command yet
            while( !commandCompletedFlag ) {
                // command input
                fOut_access.lock();
                cout << "To send train '" << trainLabel << "', enter the command 'depart':";
                cin >> command;
                fOut_access.unlock();

                // command analysis
                if (command == "depart") {
                    // conclusion about the departure of the train
                    fOut_access.lock();
                    cout << "Train '" << trainLabel << "' has departed.\n";
                    fOut_access.unlock();

                    // departed train mark
                    for (auto &t: train)
                        if (trainLabel == t.label) {
                            t.flag = true;
                            break;
                        }

                    // flag setting - the command is executed.
                    commandCompletedFlag = true;
                }// end of "if (command == "depart")"
            }// end of "while( !commandCompletedFlag )"
        }// end of "if( trainLabel != "none" )"

        // if all trains have been at the station, then exit the station maintenance cycle
        everyoneWasAtTheStation = true;
        for(auto &t: train)
            everyoneWasAtTheStation &= t.flag;
    }while( !everyoneWasAtTheStation );

    return 0;
}
