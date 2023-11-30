#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <nlohmann/json.hpp>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

using json = nlohmann::json;


// One shot RTII wrapper for reading and writing to forked subprocess
class PipeWrapper {
    int read_pipe_fd;
    int write_pipe_fd;

public:
    PipeWrapper(std::string cmd, std::string arg){
        pid_t pid;
        int pipes[4];

        int err = 0;
        /* TODO handle errors*/
        err |= pipe(&pipes[0]); /* Parent read/child write pipe */
        err |= pipe(&pipes[2]); /* Child read/parent write pipe */
        if (err != 0){
            std::cout << "Pipe failed\n";
            std::cout << strerror(errno) << "\n";
            throw std::exception();
        }

        // BIG question
        // how to handle execl error like 
        //   ---- file not found

        if ((pid = fork()) > 0) {
            /* Parent process */
            read_pipe_fd = pipes[0];
            write_pipe_fd = pipes[3];

            close(pipes[1]);
            close(pipes[2]);

        } else if (pid == 0) {
            close(pipes[0]);
            close(pipes[3]);

            dup2(pipes[1], STDOUT_FILENO);
            dup2(pipes[2], STDIN_FILENO);

            execl(cmd.c_str(), cmd.c_str(), arg.c_str(), (char*)NULL);
            std::cout << "execl failed somehow" << strerror(errno) << "\n";
            std::terminate();
        } else {
            std::cout << "FORK FAILED\n";
            throw "FORK FAILED";
        }
    }

    // closes the write pipe
    void write_to_pipe(std::string const &s){
        int len = s.length();
        int written = write(write_pipe_fd, s.c_str(), len);
        std::cout << "wrote but (" << len - written<<")\n";
        written = write(write_pipe_fd, "\n", 1);        
        std::cout << "wrote new" << written << "line\n";
        close(write_pipe_fd);
    }

    std::string read_output(){
        char buffer[1024];
        std::string result = "";

        // Read the output of the Python program
        // until it closes pipe
        while (read(read_pipe_fd, buffer, sizeof(buffer)) > 0) {
            result += buffer;
        }
        std::cout << "Read " << result.length(); 
        // no more reading
        close(read_pipe_fd);
        return result;
    }

    ~PipeWrapper(){
        // we can preform wait since the python running process
        // will have finshed by now since 
        // it's finished by read_output call finishes on read
        wait(NULL);
    }
};

// creates filler python file with desired method
// assumed linux path 
// return false on failure, else true
// this is getting pretty long
// we need to register the func with json
// create some manager class to do this
bool create_default_python_file(std::string name, std::string const& method, std::string path)
{
    // in debug:
    if (method.find(name) == std::string::npos) {
        std::cout << "Name not found in method!" << '\n';
        return false;
    }
    if (not path.ends_with("/")){
        path += "/";
    }
    path += name;
    if (not path.ends_with(".py")){
        path += ".py";
    }

    std::ofstream file(path);
    if (not file.is_open()){
        std::cout << "Couldn't create file " << errno <<std::endl;;
        return false;
    }
    
    // Template Stuff
    const std::string template_str1 =
        "def main():\n"
        "    s = sys.stdin.readline()\n"
        "    data = json.loads(s)\n"
        "    ret = {}\n"
        "    try:\n"
        "        ret['res'] = ";

    const std::string template_str2 =
        "        ret['err'] = False\n"
        "    except Exception as e:\n"
        "         ret['err'] = True\n"
        "         ret['what'] = str(e)\n"
        "    print(json.dumps(ret), end='\\n\\0')\n"    // why \n\0 ? so it doesnt fail somehow
        "main()";

    file << "import json, sys\n" <<  method << "\n";
    file << template_str1;
    file << name << "(**data)\n";
    file << template_str2 << "\n";
    
    file.close();
    return true;
}

// json run_python_file()

int main(){

    bool res = create_default_python_file(
        "add", "def add(a, b):\n    return a+b", "./python_funcs/"
    );

    if (not res){
        std::cout << "creating file failed\n";
    }


    // json availableFuncs;
    // try {
    //     std::ifstream f("python_funcs/metaData.json");
    //     availableFuncs = json::parse(f);
    // } catch(const std::exception& e) {
    //     printf("Errrorrr");
    //     std::cerr << e.what() << '\n';
    //     return -1;
    // }
    
    // for (json::iterator it = availableFuncs.begin(); it != availableFuncs.end(); ++it ){
    //     std::cout << it.key()<< "\n";
    //     // creare the python files
    //     std::cout << (*it)["args"] << "\n";
    //     std::cout << (*it)["comment"] << "\n";
    //     std::cout << (*it)["args"].is_array() << "\n";
    // }    

    json args;
    args["a"] = 1;
    args["b"] = 2;
    args["c"] = 3;

    PipeWrapper py_pipe{"/usr/bin/python3", "python_funcs/addThree.py"};
    py_pipe.write_to_pipe(args.dump());
    auto out = py_pipe.read_output();
    std::cout << out << "\n";
    json python_output = json::parse(out);
    std::cout << python_output.dump() << "\n";

    PipeWrapper py_pipe2{"/usr/bin/python3", "python_funcs/add.py"};
    py_pipe2.write_to_pipe(args.dump());
    auto out2 = py_pipe2.read_output();
    std::cout << out2 << "\n";
    json python_output2 = json::parse(out2);
    std::cout << python_output2.dump() << "\n";

}
