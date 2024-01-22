#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <set>
#include <fstream>

using namespace std;

//vector to store websites indexed(nodes of the graph where each website represents a node in the graph) in it
vector<string> websites;
int createindex(string website)
{
    for (int i = 0; i < websites.size(); i++) {
        if (websites[i] == website)
            return i;
            
    }
    websites.push_back(website);
    return websites.size() - 1;
}

//Array to read impression from file into it
int impressions[30];
void readimpressionsfromfile()
{
    fstream file;
    string line;
    file.open("impressions.csv", ios::in);
    while (getline(file, line)) {
        stringstream s(line);

        string website, impression;
        getline(s, website, ',');
        int i = createindex(website);

        getline(s, impression, ',');
        impressions[i] = stoi(impression);
    }
}

//Array to read clicks from file into it
//clicks are all initialized to be 1 in the file
int clicks[30];
void readclicksfromfile()
{
    fstream file;
    string line;
    file.open("clicks.csv", ios::in);
    while (getline(file, line))
    {
        stringstream s(line);
        string website, click;
        getline(s, website, ',');
        int i = createindex(website);

        getline(s, click, ',');
        clicks[i] = stoi(click);
    }
}

//reading the adjacency matrix from file to specify links between websites
int Graph[30][30];
void readgraphfromfile()
{
    fstream file;
    string line;
    file.open("webgraph.csv", ios::in);
    while (getline(file, line))
    {
        stringstream s(line);
        string node1, node2;
        getline(s, node1, ',');
        getline(s, node2, ',');
        int source = createindex(node1);
        int destination = createindex(node2);
        Graph[source][destination] = 1;
    }
}

//hashtable to store keywords because they will be used in the search 
set<string> keywords[100];
void readkeywordsfromfile()
{
    fstream file;
    string line;

    file.open("keyword.csv", ios::in);

    while (getline(file, line)) {
        stringstream s(line);
        string website, keyw;
        getline(s, website, ',');
        int i = createindex(website);
        while (getline(s, keyw, ',')) {
            keywords[i].insert(keyw);
        }
        cout << endl;
    }
}
// array to store in it the score of each website
float scores[30];
void computescore()
{
    //computing the page rank
    int size = websites.size();
    vector<float> initialiterations(size, 1.0 / size);
    for (int i = 0; i < 2; i++) {
        vector<float> nextiterations(size, 0);
        for (int j = 0; j < size; j++) {
            int outgoinglinks = 0;
            for (int k = 0; k < size; k++)
            {
                if (Graph[j][k] != 0)
                    outgoinglinks++;
                nextiterations[k] = nextiterations[k] + (initialiterations[j] / outgoinglinks);
            }
        }
        initialiterations = nextiterations;
    }
    //computing CTR
    for (int i = 0; i < size; i++)
    {
        float CTR = 0;
        if (impressions[i] != 0)
            CTR = (clicks[i] * 1.0) / impressions[i];

        scores[i] = 0.4 * initialiterations[i] + ((1 - (0.1 * impressions[i]) / (1 + 0.1 * impressions[i])) * initialiterations[i]
            + ((0.1 * impressions[i]) / (1 + 0.1 * impressions[i])) * CTR
            ) * 0.6;
    }
}
void searchresult(vector<pair<int, string>> results)
{
    cout << endl << "Search Results:\n";
    for (int i = 0; i < results.size(); i++)
    {
        cout << (i + 1) << ". "
            << results[i].second << endl;
    }
    cout << endl;
}
//function to divide a string into words
string getword(string word)
{
    word.erase(remove(word.begin(), word.end(), '\"'), word.end());
    return word;
}

vector<pair<int, string>> finalresult(string searchquery)
{
    string querytype = "OR";
    vector<string> words;

    if (searchquery.find(" AND ") != string::npos)
    {
        const char* c = searchquery.c_str();
        char* word = strtok((char*)c, " AND ");
        while (word) {
            string s(word);
            s = getword(s);
            if (s != "") {
                words.push_back(s);
            }
            word = strtok(NULL, " AND ");
        }
        querytype = "AND";
    }

    else if (searchquery.find("OR") != string::npos)
    {
        const char* c = searchquery.c_str();
        char* word = strtok((char*)c, " OR ");
        while (word) 
        {
            string s(word);
            s = getword(s);

            if (s != "") {
                words.push_back(s);
            }
            word = strtok(NULL, " OR ");
        }
        querytype = "OR";
    }
    else
    {
        const char* c = searchquery.c_str();
        char* word = strtok((char*)c, " ");
        while (word) 
        {
            string s(word);
            s = getword(s);
            if (s != "") {
                words.push_back(s);
            }
            word = strtok(NULL, " ");
        }
        querytype = "OR";
    }
     vector<pair<int, string>> results;

    if (querytype == "AND") 
    {
        for (int i = 0; i < websites.size(); i++) {
            bool included = true;

            for (int j = 0; j < words.size(); j++) {
                if (keywords[i].find(words[j]) == keywords[i].end()) {
                    included = false;
                    break;
                }
            }
            if (included) {
                results.push_back(make_pair(scores[i], websites[i]));
            }
        }
    }
    else {
        for (int i = 0; i < websites.size(); i++) {
            bool included = false;

            for (int j = 0; j < words.size(); j++) {
                if (keywords[i].find(words[j]) != keywords[i].end()) 
                {
                    included = true;
                    break;
                }
            }

            if (included) 
            {
                results.push_back(make_pair(scores[i], websites[i]));
            }
        }
    }

    sort(results.rbegin(), results.rend());
    searchresult(results);
    return results;
}

void impressionsincrement(vector<pair<int, string>> results)
{
    for (int i = 0; i < results.size(); i++)
    {
        impressions[createindex(results[i].second)]++;
    }
}

void clicksincrement(string line)
{
    clicks[createindex(line)]++;
}

void impressionsupdateinfile()
{
    ofstream theFile("impressions.csv");
    for (int i = 0; i < websites.size(); i++) {
        theFile << websites[i] << "," << impressions[i] << "\n";
    }
    theFile.close();
}

void clicksupdateinfile()
{
    ofstream theFile("clicks.csv");
    for (int i = 0; i < websites.size(); i++)
    {
        theFile << websites[i] << "," << clicks[i] << "\n";
    }
    theFile.close();
}
int menu1()
{
    int answer;
    cout << "welcome! \n";
    cout << "What would you like to do? \n";
    cout << "1. New Search \n";
    cout << "2. Exit \n";
    cout << endl;
    cout << "please enter your Choice: ";
    string line;
    getline(cin, line);
    return stoi(line);
}
int menu2()
{
    int answer;
    cout << "Would you like to: \n";
    cout << "1. Choose a web page to open \n";
    cout << "2. New Search \n";
    cout << "3. Exit \n";
    cout << endl;
    cout << "please enter your Choice:";
    string line;
    getline(cin, line);
    return stoi(line);
}
int menu3()
{
    int answer;
    cout << "Would you like to: \n";
    cout << "1. Back to Search Results \n";
    cout << "2. New Search \n";
    cout << "3. Exit \n";
    cout << endl;
    cout << "please enter your Choice:";
    string line;
    getline(cin, line);
    return stoi(line);
}
int main()
{
    readgraphfromfile();
    readkeywordsfromfile();
    readimpressionsfromfile();
    readclicksfromfile();
    computescore();
    int answer;
    string usersquery;
    string line;
    answer = menu1();
    while (answer != 2)
    {
        cout << "Enter Search Query: ";
        getline(cin, usersquery);
        vector<pair<int, string>> results = finalresult(usersquery);
        impressionsincrement(results);
        int answer2 = menu2();
        while (answer2 == 1)
        {
            searchresult(results);
            cout << "choose a page to search: ";
            string line;
            getline(cin, line);
            int i = stoi(line);
            cout << "The page you are now viewing is" << results[i - 1].second << endl << endl;
            clicksincrement(results[i - 1].second);
            answer2 = menu3();
        }
        if (answer2 == 3)
        {
            break;
        }
    }
    impressionsupdateinfile();
    clicksupdateinfile();
}