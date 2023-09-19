#ifndef STELLAR_ARENA_ENGINE_H
#define STELLAR_ARENA_ENGINE_H

#include <queue>

class Engine {
  public:
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(const char *file);

    ~Engine();

    const std::string &get_name(void) const { return name; }
    const std::string &get_author(void) const { return author; }

    void send(std::string &&command);
    std::string receive(void);

  private:
    [[noreturn]] void start_engine(void);

    const char *file = nullptr;
    int fd_to[2], fd_from[2];
    pid_t engine_pid;

    std::string name, author;

    char rb[1000];
    int rb_size = 0;

    std::queue<std::string> q;
    std::string q_buffer;

    static uint16_t id_t;
    uint16_t id = id_t++;
};

#endif
