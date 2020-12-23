#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <vector>


//#define DEBUG

const char* path_to_file = "result.out";
//const char* path_to_file = "/home/box/result.out";
//const char *path_to_file = "/home/maks/box/result.out";

// pipe
// [0] - read
// [1] - write

char *TrimWhiteSpace(char *str);

void process(
    std::vector<std::pair<std::string, char *const *>> &input_commands,
    int command_number);

void process(
    std::vector<std::pair<char *, char *const *>> &commands,
    int command_number,
    int *fd) {

#ifdef DEBUG
  std::cerr << "***" << std::endl;
  std::cerr << "command_number = " << command_number << std::endl;
  std::cerr << "command: " << commands[command_number].first << std::endl;
#endif

  size_t size = commands.size();

  if (command_number == (size - 1)) {

#ifdef DEBUG
    std::cerr << "*******" << std::endl;
    std::cerr << "Last command!" << std::endl;
#endif

    int fd_out = open(path_to_file, O_RDWR | O_CREAT | O_TRUNC, 0666);

    close(STDOUT_FILENO);
    dup2(fd_out, STDOUT_FILENO);

#ifdef DEBUG
    std::cerr << "command: " << commands[command_number].first << std::endl;
#endif

    execvp(commands[command_number].first,
           commands[command_number].second);
  } else {

#ifdef DEBUG
    std::cerr << "Not last command!" << std::endl;
#endif

    pipe(fd);

    if (fork()) {
      close(STDIN_FILENO);
      dup2(fd[0], STDIN_FILENO);
      close(fd[0]);
      close(fd[1]);

      int fd_2[2];

      process(commands, command_number + 1, fd_2);

      return;
    } else {
      close(STDOUT_FILENO);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[0]);
      close(fd[1]);

#ifdef DEBUG
      std::cerr << "*****" << std::endl;
      std::cerr << "parent command_number = " << command_number << std::endl;
      std::cerr << "parent command: " << commands[command_number].first << std::endl;
#endif

      execvp(commands[command_number].first,
             commands[command_number].second);
    }
  }
}


void my_bash(std::vector<std::pair<char *, char *const *>> &commands) {
  size_t size = commands.size();

  int fd[2];

  process(commands, 0, fd);
}

/// Note: This function returns a pointer to a substring of the original string.
/// If the given string was allocated dynamically, the caller must not overwrite
/// that pointer with the returned value, since the original pointer must be
/// deallocated using the same allocator with which it was allocated.  The return
/// value must NOT be deallocated using free() etc.
/// Link: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *TrimWhiteSpace(char *str) {
  char *end;

  // Trim leading space
  while (isspace(*str)) str++;

  if (*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end + 1) = 0;

  return str;
}

void parse_command(const std::string &command,
                   std::string &name,
                   std::string &argc) {

  std::string::size_type pos = command.find_first_of(' ');

  if (pos != std::string::npos) {
    name = command.substr(0, pos);
    argc = command.substr(pos + 1, command.length() - pos - 1);
  } else {
    name = command;
    argc = "";
  }
}

void parse_argc(const std::string &argv,
                std::vector<std::string> &argc_vector) {
  std::string tmp(argv);

  std::string::size_type pos = tmp.find_first_of(' ');

  while (pos != std::string::npos) {
    std::string argc = tmp.substr(0, pos);

    tmp = tmp.substr(pos + 1, tmp.length() - pos - 1);

    argc_vector.push_back((argc));

    pos = tmp.find_first_of(' ');
  }

  if (tmp != "") {
    argc_vector.push_back((tmp));
  }
}

int main() {
//  ls | sort | wc -l
//  ls | sort

  std::vector<std::string> input_commands;

  // Store input data.
  char *input_data = {0};
  // For buffer.
  char buffer_data[256] = {0};

  //printf("Input data: ");
  input_data = fgets(buffer_data, 256, stdin);

  int count_sentence = 1;

  int i = 0;

  // Calculate how many sentences we have.
  while (input_data[i]) {

    // Compare with ascii dot number.
    if (input_data[i] == '|') {
      count_sentence++;
    }

    i++;
  }


  char *current = NULL;
  char delimeter[] = "|";

  current = strtok(input_data, delimeter);

  // Parse input string with help of strtok function.
  while (current != NULL) {
    current = TrimWhiteSpace(current);

    input_commands.push_back(current);

    current = strtok(NULL, delimeter);
  }

  std::vector<std::pair<char *, char *const *>> commands;

  for (const auto &item : input_commands) {

#ifdef DEBUG
    std::cout << item << std::endl;
#endif

    std::vector<char *> args;

    char *input_data_args = strdup(item.c_str());

    char *current_arg = NULL;
    char delimeter_arg[] = " ";

    current_arg = strtok(input_data_args, delimeter_arg);

    // Parse input string with help of strtok function.
    while (current_arg != NULL) {
      current_arg = TrimWhiteSpace(current_arg);

      args.push_back(current_arg);

      current_arg = strtok(NULL, delimeter_arg);
    }

    args.push_back(NULL);

    size_t argc_size = args.size();

    char **arcv_char = new char *[argc_size];

    for (size_t i = 0; i < argc_size; i++) {
      arcv_char[i] = args[i];
    }

    commands.push_back(std::make_pair(arcv_char[0], arcv_char));
  }

  my_bash(commands);

  return 0;
}
