#pragma once

#include <sys/select.h>
#include <list>

namespace FlyZero {
    class fdset : public fd_set {
    public:
        fdset(void) {
            zero();
        }

        void zero(void) {
            for (unsigned int i = 0; i < MAX_FD_NUMBER; ++i)
                fd[i] = -1;
            FD_ZERO(&saved);
            nfd = 0;
        }

        void set(int fd) {
            FD_SET(fd, &saved);
            maxfdp1 = maxfdp1 > fd ? maxfdp1 : (fd + 1);
            ++nfd;
        }

        int isset(int fd) {
            return FD_ISSET(fd, this);
        }

        void clear(int fd) {
            return FD_CLR(fd, &saved);
            for (unsigned int i = 0; i < MAX_FD_NUMBER; ++i) {
                //			if (this->fd[i] == fd)
            }
        }

        unsigned int get_fd_number(void) const {
            return nfd;
        }

        int get_maxfdp1(void) const {
            return maxfdp1;
        }

        void restore(void) {
            *((fd_set *)(this)) = saved;
        }

        void save(void) {
            saved = *this;
        }

    private:
        bool add(int fd) {
            if (nfd >= MAX_FD_NUMBER)
                return false;
            else for (unsigned int i = 0; i < MAX_FD_NUMBER; ++i) {
                if (this->fd[i] == -1) {

                }
            }
        }

        void remove(int fd) {

        }

    private:
        static const unsigned int MAX_FD_NUMBER{ sizeof (fd_set) * 8 };
        int fd[MAX_FD_NUMBER];
        std::list<int> fd_list;
        unsigned int nfd;
        fd_set saved;
        int maxfdp1;
    };
}
