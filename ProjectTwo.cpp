// Justin White
// CS 300 Project Two
// Load course records into a binary search tree for listing and lookup.

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Course {
    string courseNumber;
    string courseName;
    vector<string> prerequisites;
};

struct TreeNode {
    Course course;
    TreeNode* left;
    TreeNode* right;
};

struct BinarySearchTree {
    TreeNode* root = nullptr;
};

string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string toUpper(string text) {
    for (char& character : text) {
        character = static_cast<char>(
            toupper(static_cast<unsigned char>(character)));
    }
    return text;
}

// Keep empty CSV fields so that blank prerequisites can be skipped safely.
vector<string> splitCourseLine(const string& line) {
    vector<string> fields;
    string field;
    stringstream lineStream(line);

    while (getline(lineStream, field, ',')) {
        fields.push_back(trim(field));
    }
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }
    return fields;
}

bool containsCourseNumber(const vector<string>& courseNumbers,
    const string& requestedCourseNumber) {
    for (const string& courseNumber : courseNumbers) {
        if (courseNumber == requestedCourseNumber) {
            return true;
        }
    }
    return false;
}

vector<Course> parseAndValidateCourses(const string& filename) {
    vector<vector<string>> parsedRows;
    vector<Course> parsedCourses;
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cout << "Error: Unable to open the course file.\n";
        return parsedCourses;
    }

    string line;
    size_t lineNumber = 0;
    while (getline(inputFile, line)) {
        ++lineNumber;
        vector<string> fields = splitCourseLine(line);

        if (fields.size() < 2) {
            cout << "Skipped line " << lineNumber
                << ": a course number and title are required.\n";
            continue;
        }
        if (fields[0].empty() || fields[1].empty()) {
            cout << "Skipped line " << lineNumber
                << ": a course number and title are required.\n";
            continue;
        }

        fields[0] = toUpper(fields[0]);
        for (size_t field = 2; field < fields.size(); ++field) {
            fields[field] = toUpper(fields[field]);
        }
        parsedRows.push_back(fields);
    }
    inputFile.close();

    // Store row positions to avoid copying all the fields on each pass.
    vector<size_t> validRows;
    for (size_t index = 0; index < parsedRows.size(); ++index) {
        validRows.push_back(index);
    }

    bool aRowWasRemoved;
    do {
        vector<string> validCourseNumbers;
        vector<size_t> nextRows;

        for (size_t rowIndex : validRows) {
            validCourseNumbers.push_back(parsedRows[rowIndex][0]);
        }

        for (size_t rowIndex : validRows) {
            const vector<string>& row = parsedRows[rowIndex];
            bool rowIsValid = true;

            // No prerequisite fields means this loop runs zero times.
            for (size_t field = 2; field < row.size(); ++field) {
                const string& prerequisiteNumber = row[field];
                if (prerequisiteNumber.empty()) {
                    continue;
                }
                if (prerequisiteNumber == row[0] ||
                    !containsCourseNumber(validCourseNumbers,
                        prerequisiteNumber)) {
                    cout << "Skipped " << row[0]
                        << ": invalid prerequisite "
                        << prerequisiteNumber << ".\n";
                    rowIsValid = false;
                    break;
                }
            }

            if (rowIsValid) {
                nextRows.push_back(rowIndex);
            }
        }

        // Repeat so no retained course depends on a rejected course.
        aRowWasRemoved = nextRows.size() < validRows.size();
        validRows = nextRows;
    } while (aRowWasRemoved);

    for (size_t rowIndex : validRows) {
        const vector<string>& row = parsedRows[rowIndex];
        Course currentCourse;
        currentCourse.courseNumber = row[0];
        currentCourse.courseName = row[1];

        for (size_t field = 2; field < row.size(); ++field) {
            if (!row[field].empty()) {
                currentCourse.prerequisites.push_back(row[field]);
            }
        }
        parsedCourses.push_back(currentCourse);
    }
    return parsedCourses;
}

Course findCourse(const BinarySearchTree& tree,
    const string& requestedCourseNumber) {
    const TreeNode* current = tree.root;
    while (current != nullptr) {
        if (requestedCourseNumber == current->course.courseNumber) {
            return current->course;
        }
        if (requestedCourseNumber < current->course.courseNumber) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }
    return Course{};
}

bool insertCourse(BinarySearchTree& tree, const Course& currentCourse) {
    TreeNode* newNode = new TreeNode{ currentCourse, nullptr, nullptr };
    if (tree.root == nullptr) {
        tree.root = newNode;
        return true;
    }

    TreeNode* current = tree.root;
    while (true) {
        if (currentCourse.courseNumber == current->course.courseNumber) {
            cout << "Skipped duplicate course "
                << currentCourse.courseNumber << ".\n";
            delete newNode;
            return false;
        }
        if (currentCourse.courseNumber < current->course.courseNumber) {
            if (current->left == nullptr) {
                current->left = newNode;
                return true;
            }
            current = current->left;
        }
        else {
            if (current->right == nullptr) {
                current->right = newNode;
                return true;
            }
            current = current->right;
        }
    }
}

// Visit left subtree, current course, then right subtree for sorted output.
void printInOrder(const TreeNode* node) {
    if (node == nullptr) {
        return;
    }
    printInOrder(node->left);
    cout << node->course.courseNumber << ", "
        << node->course.courseName << '\n';
    printInOrder(node->right);
}

void searchCourse(const BinarySearchTree& tree,
    const string& requestedCourseNumber) {
    Course selectedCourse = findCourse(tree, requestedCourseNumber);
    if (selectedCourse.courseNumber.empty()) {
        cout << "Course not found.\n";
        return;
    }

    cout << selectedCourse.courseNumber << ", "
        << selectedCourse.courseName << '\n';
    cout << "Prerequisites: ";
    if (selectedCourse.prerequisites.empty()) {
        cout << "None";
    }
    else {
        for (size_t index = 0;
            index < selectedCourse.prerequisites.size(); ++index) {
            if (index > 0) {
                cout << ", ";
            }
            cout << selectedCourse.prerequisites[index];
        }
    }
    cout << '\n';
}

// Release every allocated node when reloading data or exiting.
void clearTree(TreeNode* node) {
    if (node == nullptr) {
        return;
    }
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

size_t loadCourses(const string& filename, BinarySearchTree& tree) {
    vector<Course> parsedCourses = parseAndValidateCourses(filename);
    if (parsedCourses.empty()) {
        cout << "No valid courses were loaded from that file.\n";
        return 0;
    }

    BinarySearchTree newTree;
    size_t loadedCount = 0;
    for (const Course& currentCourse : parsedCourses) {
        if (insertCourse(newTree, currentCourse)) {
            ++loadedCount;
        }
    }

    // Replace the previous tree only after the new data is ready.
    clearTree(tree.root);
    tree.root = newTree.root;
    cout << loadedCount << " courses loaded.\n";
    return loadedCount;
}

int main() {
    BinarySearchTree tree;
    bool dataLoaded = false;
    string menuChoice;

    cout << "Welcome to the course planner.\n";
    while (true) {
        cout << "\n1. Load Data Structure.\n"
            << "2. Print Course List.\n"
            << "3. Print Course.\n"
            << "9. Exit\n\n"
            << "What would you like to do? ";

        if (!getline(cin, menuChoice)) {
            break;
        }
        menuChoice = trim(menuChoice);

        if (menuChoice == "1") {
            string filename;
            cout << "Enter the full path and file name: ";
            if (!getline(cin, filename)) {
                break;
            }
            filename = trim(filename);
            // Windows Copy as path may surround the path with quotes.
            if (filename.size() >= 2 && filename.front() == '"' &&
                filename.back() == '"') {
                filename = filename.substr(1, filename.size() - 2);
            }
            if (loadCourses(filename, tree) > 0) {
                dataLoaded = true;
            }
        }
        else if (menuChoice == "2") {
            if (!dataLoaded) {
                cout << "Load course data first.\n";
            }
            else {
                cout << "Here is a sample schedule:\n\n";
                printInOrder(tree.root);
            }
        }
        else if (menuChoice == "3") {
            if (!dataLoaded) {
                cout << "Load course data first.\n";
            }
            else {
                string requestedCourseNumber;
                cout << "What course do you want to know about? ";
                if (!getline(cin, requestedCourseNumber)) {
                    break;
                }
                requestedCourseNumber = toUpper(trim(requestedCourseNumber));
                searchCourse(tree, requestedCourseNumber);
            }
        }
        else if (menuChoice == "9") {
            break;
        }
        else if (menuChoice.empty()) {
            cout << "Invalid option. Please enter 1, 2, 3, or 9.\n";
        }
        else {
            cout << menuChoice << " is not a valid option.\n";
        }
    }

    clearTree(tree.root);
    cout << "Thank you for using the course planner!\n";
    return 0;
}