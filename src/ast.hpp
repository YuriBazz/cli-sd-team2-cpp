#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>

class Environment;

struct Argument {
    enum Type { WORD, STRING, VARIABLE, COMPOSITE };
    Type type;
    std::string value;
    std::vector<Argument*> parts;
    Argument(Type t, const char* v) : type(t), value(v ? v : "") {}
    Argument(Type t, const std::vector<Argument*>& p) : type(t), parts(p) {}
    ~Argument() {
        if (type == COMPOSITE) {
            for (auto p : parts) delete p;
        }
    }
};

using ArgumentList = std::vector<Argument*>;

class Statement {
   public:
    virtual ~Statement() = default;
    virtual void execute(Environment& env) = 0;
};

class Assignment : public Statement {
   public:
    std::string varName;
    ArgumentList* args;
    Assignment(const std::string& name, ArgumentList* a) : varName(name), args(a) {}
    ~Assignment() {
        if (args) {
            for (auto a : *args) delete a;
            delete args;
        }
    }
    void execute(Environment& env) override;
};

class Command : public Statement {
   public:
    ArgumentList* args;
    Command(ArgumentList* a) : args(a) {}
    virtual ~Command() {
        if (args) {
            for (auto a : *args) delete a;
            delete args;
        }
    }
    virtual void execute(Environment& env, int inputFd = 0, int outputFd = 1) = 0;
    void execute(Environment& env) override { execute(env, 0, 1); }
};

class Pipeline : public Statement {
   public:
    std::vector<Command*> commands;
    explicit Pipeline(const std::vector<Command*>& cmds) : commands(cmds) {}
    ~Pipeline() {
        for (auto c : commands) delete c;
    }
    void execute(Environment& env) override;
};

class CatCommand : public Command {
   public:
    explicit CatCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class EchoCommand : public Command {
   public:
    explicit EchoCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class WcCommand : public Command {
   public:
    explicit WcCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class PwdCommand : public Command {
   public:
    explicit PwdCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class ExitCommand : public Command {
   public:
    explicit ExitCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class GrepCommand : public Command {
   public:
    explicit GrepCommand(ArgumentList* a) : Command(a) {}
    void execute(Environment& env, int inputFd, int outputFd) override;
};

class LsCommand : public Command {
   public:
    void execute(Environment& env, int inputFd, int outputFd) override;
    explicit LsCommand(ArgumentList* a) : Command(a) {}
};

class ExternalCommand : public Command {
   public:
    ExternalCommand(const std::string& cmd, ArgumentList* a) : Command(a), command(cmd) {}
    void execute(Environment& env, int inputFd, int outputFd) override;

   private:
    std::string command;
    std::string findInPath(const std::string& cmd, const Environment& env);
};

#endif
