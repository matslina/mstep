#pragma once

class Mode {
public:
  virtual void start() {}
  virtual void stop() {}
  virtual bool tick() = 0;
};
