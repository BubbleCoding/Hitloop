#ifndef PROCESS_H
#define PROCESS_H

class Process {
public:
    virtual ~Process() {}
    virtual void setup() {}
    virtual void update() = 0;
};

#endif // PROCESS_H 