#include <vector> // vector, push_back, at
#include <string> // string
#include <iostream> // cin, getline
#include <fstream> // ofstream
#include <unistd.h> // getopt, exit, EXIT_FAILURE
#include <assert.h> // assert
#include <thread> // thread, join
#include <sstream> // stringstream
#include <string.h>

#include "BoundedBuffer.h" // BoundedBuffer class

#define MAX_MSG_LEN 256

using namespace std;

/************** Helper Function Declarations **************/

void parse_column_names(vector<string>& _colnames, const string& _opt_input);
void write_to_file(const string& _filename, const string& _text, bool _first_input=false);

/************** Thread Function Definitions **************/

// "primary thread will be a UI data entry point"
void ui_thread_function(BoundedBuffer* bb) {
    // TODO: implement UI Thread Function
    // infinite loop -> prompt user for input
    //  if Exit -> break
    //  else -> push input onto bb
    while (true){
        char message[MAX_MSG_LEN];
        cin >> message;
        if (strcmp(message, "Exit") == 0){
            // cout << "break" << endl;
            // bb -> push((char*)message, (int)sizeof(message));
            break;
        }
        else {
            bb ->push((char*)message, (int)sizeof(message));
        }
    }
}

// "second thread will be the data processing thread"
// "will open, write to, and close a csv file"
void data_thread_function(BoundedBuffer* bb, string filename, const vector<string>& colnames) {
    // TODO: implement Data Thread Function
    // (NOTE: use "write_to_file" function to write to file)
    // infinite loop -> pop from bb
    //  if exit packet -> break (compare to exit)
    //  else -> call write_to_file
    int count = 1;

    while (true){
        char message[MAX_MSG_LEN];
        bb ->pop(message, MAX_MSG_LEN);
        if (strcmp(message, "Exit") == 0){
            // cout << "break2" << endl;
            break;
        }
        else {
            if (count == 1){
                write_to_file(filename, message, 0);
                if ((int)colnames.size() == 1){
                    write_to_file(filename, "\n", 0);
                }
                else{
                    write_to_file(filename, ", ", 0);
                    count ++;
                }
            }
            else if (count == (int)colnames.size()){
                write_to_file(filename, message, 0); 
                write_to_file(filename, "\n", 0);
                count = 1;
            }
            else {
                write_to_file(filename, message, 0);
                write_to_file(filename, ", ", 0);
                count++;
            }
        }
    }
}

/************** Main Function **************/

int main(int argc, char* argv[]) {

    // variables to be used throughout the program
    vector<string> colnames; // column names
    string fname; // filename
    BoundedBuffer* bb = new BoundedBuffer(3); // BoundedBuffer with cap of 3

    // read flags from input
    int opt;
    while ((opt = getopt(argc, argv, "c:f:")) != -1) {
        switch (opt) {
            case 'c': // parse col names into vector "colnames"
                parse_column_names(colnames, optarg);
                break;
            case 'f':
                fname = optarg;
                break;
            default: // invalid input, based on https://linux.die.net/man/3/getopt
                fprintf(stderr, "Usage: %s [-c colnames] [-f filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // create file of fname with column headers of colnames
    // first time creating columns make _first_input == 1; else make it 0
    for (size_t i = 0; i < colnames.size(); i++){
        if (i == 0 && colnames.size() == 1){ // beginning of the file and only 1 colname
            write_to_file(fname, colnames.at(i) + "\n", 1);
        }
        else if (i == 0 && colnames.size() != 1){
            write_to_file(fname, colnames.at(i), 1);
        }
        else if (i != colnames.size() - 1) { // middle of the line
            write_to_file(fname, ", " + colnames.at(i), 0);
        }
        else{ // end of the line
            write_to_file(fname, ", " + colnames.at(i) + "\n", 0);
        }
    }

    //ui_thread_function(bb);
    //data_thread_function(bb, fname, colnames);

    // thread vectors
    vector<thread> ui_threads;
    vector<thread> data_threads;
    // TODO: instantiate ui and data threads
    ui_threads.push_back(thread(ui_thread_function, bb));
    data_threads.push_back(thread(data_thread_function, bb, fname, colnames));
    
    // TODO: join ui_thread
    ui_threads.at(0).join();

    // TODO: "Once the user has entered 'Exit', the main thread will
    // "send a signal through the message queue to stop the data thread"
    char qm[MAX_MSG_LEN] = "Exit";
    bb ->push(qm, MAX_MSG_LEN);

    // TODO: join data thread
    data_threads.at(0).join();

    // CLEANUP: delete members on heap
    delete bb;
}

/************** Helper Function Definitions **************/

// function to parse column names into vector
// input: _colnames (vector of column name strings), _opt_input(input from optarg for -c)
void parse_column_names(vector<string>& _colnames, const string& _opt_input) {
    stringstream sstream(_opt_input);
    string tmp;
    while (sstream >> tmp) {
        _colnames.push_back(tmp);
    }
}

// function to append "text" to end of file
// input: filename (name of file), text (text to add to file), first_input (whether or not this is the first input of the file)
void write_to_file(const string& _filename, const string& _text, bool _first_input) {
    // based on https://stackoverflow.com/questions/26084885/appending-to-a-file-with-ofstream
    // open file to either append or clear file
    ofstream ofile;
    if (_first_input)
        ofile.open(_filename);
    else
        ofile.open(_filename, ofstream::app);
    if (!ofile.is_open()) {
        perror("ofstream open");
        exit(-1);
    }

    // sleep for a random period up to 5 seconds
    usleep(rand() % 5000);

    // add data to csv
    ofile << _text;

    // close file
    ofile.close();
}