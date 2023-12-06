#ifndef STELLAR_ARENA_ENGINE_H
#define STELLAR_ARENA_ENGINE_H

#include <queue>

class Engine {
  public:
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(const char *file);

    ~Engine();

    [[nodiscard]] const std::string &get_name() const { return name; }
    [[nodiscard]] const std::string &get_author() const { return author; }

    void send(std::string &&command);
    std::string receive();

  private:
    class Pipes {
      public:
        Pipes();
        Pipes(const Pipes &) = delete;
        Pipes &operator=(const Pipes &) = delete;

        ~Pipes() {
            if (!closed) close();
        }

        void close();

        int operator[](int idx) { return fd[idx]; }

      private:
        int fd[2]{};
        bool closed = false;
    };

    [[noreturn]] void start_engine();

    const char *file = nullptr;
    Pipes pipeFrom, pripeTo;
    pid_t engine_pid;

    std::string name, author;

    char rb[1000]{};
    int rb_size = 0;

    std::queue<std::string> q;
    std::string q_buffer;

    static uint16_t id_t;
    uint16_t id = id_t++;
};

#endif
