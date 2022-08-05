#include <iostream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;



// data

// data for threads
string railwayStation;
mutex railwayStation_access;

// stopping point number 1
mutex stopPoint_1;

// stopping point number 2
mutex stopPoint_2;

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

    // we are trying to pass the breakpoint number 2 - we are waiting until the already occupied space is freed up.
    stopPoint_2.lock();

    // write down the designation of the train arriving at the station
    railwayStation_access.lock();
    railwayStation = label;
    railwayStation_access.unlock();

    // message output
    fOut_access.lock();
    cout << "Train '" << label << "' arrived at the station.\n";
    fOut_access.unlock();

    // trying to pass breakpoint #1 - waiting for the 'depart' command to execute.
    stopPoint_1.lock();

    // conclusion about the departure of the train
    fOut_access.lock();
    cout << "Train '" << label << "' has departed.\n";
    fOut_access.unlock();
}



int main() {
    // data

    // train data
    const int N = 3;
    struct Train {
        string label;             // train symbol
        int time = 0;             // train travel time
        thread* track = nullptr;  // thread object
    } train[N];
    // flag
    bool everyoneWasAtTheStation;
    // train label
    string trainLabel;




    // data input
    for(auto &t: train) {
        cout << "Enter label:";
        cin >> t.label;
        cout << "Enter time:";
        cin >> t.time;
    }
    cout << endl;

    // preparation of the station for the arrival of the train
    stopPoint_1.lock();
    railwayStation = "";

    // start threads
    for(auto& t: train)
        t.track = new thread(thread_train, t.label, t.time);

    // station work
    do{
        // pause (poll the queue twice per second)
        this_thread::sleep_for(chrono::milliseconds(500));

        // get value from queue
        railwayStation_access.lock();
        trainLabel = railwayStation;
        railwayStation_access.unlock();

        // if it is a train designation
        if( trainLabel.length() != 0 ) {
            // data
            string command;
            bool commandCompletedFlag;

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
                    // departed train mark
                    // unblocking a breakpoint to send a train
                    stopPoint_1.unlock();
                    // waiting for the thread to complete
                    for (auto &t: train)
                        if (trainLabel == t.label) {
                            t.track->join();
                            t.track = nullptr;
                            break;
                        }

                    // when the train is sent, we make preparations for the arrival of the next train
                    // clearing the train marker
                    railwayStation_access.lock();
                    railwayStation = "";
                    railwayStation_access.unlock();
                    // and unblock breakpoint #2 to allow the arrival of the next train
                    stopPoint_2.unlock();

                    // flag setting - the command is executed.
                    commandCompletedFlag = true;
                }// end of "if (command == "depart")"
            }// end of "while( !commandCompletedFlag )"
        }// end of "if( trainLabel != "none" )"

        // if all trains have been at the station, then exit the station maintenance cycle
        everyoneWasAtTheStation = true;
        for(auto &t: train)
            everyoneWasAtTheStation &= (t.track == nullptr);
    }while( !everyoneWasAtTheStation );

    // data reset
    stopPoint_1.unlock();
    railwayStation = "";

    return 0;
}
