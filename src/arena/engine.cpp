#include "engine.hpp"
#include "logger.hpp"

#include <sstream>

#include <sys/wait.h>
#include <unistd.h>


uint16_t Engine::id_t = 0;
Engine::Engine(const char *file) : file(file) {
    if (pipe(fd_to) < 0 || pipe(fd_from) < 0) {
        logger::error("pipe");
        throw std::runtime_error("pipe failed");
    }

    if ((engine_pid = fork()) > 0) {
        start_engine();
    } else if (engine_pid < 0) {
        logger::error("fork");
        throw std::runtime_error("fork failed");
    }

    if (close(fd_to[0]) < 0 || close(fd_from[1])) {
        logger::error("close");
        throw std::runtime_error("close failed");
    }

    send("uci");

    logger::log(std::format("Engine {}: waiting for uciok from {}", id, file));
    while (true) {
        std::string tmp, response = receive();
        if (response == "uciok") break;
        std::stringstream ss(response);
        ss >> tmp >> tmp;
        ss.ignore(1);
        if (tmp == "name") getline(ss, name);
        if (tmp == "author") getline(ss, author);
    }
    logger::log(std::format("Engine {}: {} is {} by {}", id, file, name, author));
    logger::log(std::format("Engine {}: created", id), logger::Debug);
}

Engine::~Engine() {
    send("quit");
    waitpid(engine_pid, nullptr, 0);
    // kill(engine_pid, SIGKILL);

    if (close(fd_to[1]) < 0) logger::error("close");
    if (close(fd_from[0]) < 0) logger::error("close");
    logger::log("Engine: destroyed", logger::Debug);
}

void Engine::send(std::string &&command) {
    command.push_back('\n');
    const char *buffer = command.data();
    size_t to_write = command.size();
    while (to_write) {
        ssize_t size = write(fd_to[1], buffer, to_write);
        if (size == -1) {
            logger::error("write");
            throw std::runtime_error("write failed");
        }
        buffer += size, to_write -= size;
    }
    command.pop_back();
    logger::log(std::format("Engine {}: TO {}: {}", id, name.size() ? name : file, command), logger::Info);
}

std::string Engine::receive() {
    int size = 0;

    while (true) {
        if (!q.empty()) {
            std::string response = q.front();
            logger::log(std::format("Engine {}: FROM {}: {}", id, name.size() ? name : file, response),
                        logger::Info);
            q.pop();
            return response;
        }

        if ((size = read(fd_from[0], rb + rb_size, sizeof(rb) - rb_size)) == -1) {
            logger::error("read");
            throw std::runtime_error("read failed");
        }

        int last = 0;
        for (int i = rb_size; i < rb_size + size; i++) {
            if (rb[i] == '\n') {
                q.push(q_buffer);
                q_buffer.clear();
                last = i;
                continue;
            }
            q_buffer += rb[i];
        }

        rb_size += size;
        if (last) {
            rb_size -= last + 1;
            memcpy(rb, rb + last + 1, rb_size);
        }
    }
}

[[noreturn]] void Engine::start_engine() {
    if (close(fd_to[1]) < 0 || close(fd_from[0])) {
        logger::error("close");
        throw std::runtime_error("close failed");
    }

    if (dup2(fd_to[0], 0) < 0 || dup2(fd_from[1], 1) < 0) {
        logger::error("dup2");
        throw std::runtime_error("dup2 failed");
    }

    execl(file, file, (char *)nullptr);
    logger::error("execl");
    throw std::runtime_error("execl failed");
}
