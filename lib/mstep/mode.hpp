#ifndef MODE_HPP_49a8dfuas48fjasefjaijf
#define MODE_HPP_49a8dfuas48fjasefjaijf


class Mode {
public:
  virtual void start() {}
  virtual void stop() {}
  virtual bool tick() = 0;
};

#endif
