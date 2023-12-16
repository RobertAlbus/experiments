#include <string>
#include <vector>

int main() {
    int index = 3;

    std::vector<std::string> strings {
        "first",
        "second",
        "third",
        "forth",
    };

    int* index_p = &index;
    char* index_char_p = reinterpret_cast<char*>(index_p);
    int* index_int_p = reinterpret_cast<int*>(index_char_p);

    std::string &str = strings[*index_int_p];

    printf("\nindexing the vector at the %s position", str.c_str());
}