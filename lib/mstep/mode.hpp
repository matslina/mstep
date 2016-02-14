#ifndef MODE_HPP_49a8dfuas48fjasefjaijf
#define MODE_HPP_49a8dfuas48fjasefjaijf

class Mode {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual unsigned int tick() = 0;
private:
};

#endif
