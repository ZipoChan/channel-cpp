#pragma once
#include<thread>
#include<string>
#include <iostream>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>
using namespace std;

template <class T>
class Sudog;

template <class T>
class Sudog_List;

enum CaseType {
    NONE,
    SEND,
    RECEIVE,
    DEFAULT
};

template<class T>
class Internal_Channel { 
public:
  unsigned int qcount;
  unsigned int dataqsiz;
  bool closed;

  deque<T> buf;
  //deque<shared_ptr<Sudog<T>>> recvq;
  //deque<shared_ptr<Sudog<T>>> sendq;
  Sudog_List<T> recvq;
  Sudog_List<T> sendq;

  mutex lock;

  Internal_Channel<T>(int bufsize);
  Internal_Channel<T>(Internal_Channel<T> &ic);

  T recv();
  void recv(T &elem);

  void send(const T& elem);
  void send(T&& elem);

  bool try_send();
  bool try_receive();

  void close(void);
};

template<class T>
class Channel {
public:
  shared_ptr<Internal_Channel<T>> ptr;

  Channel<T>(const int& bufsize);

  T recv(void);
  void recv(T &elem);

  void send(const T& elem);
  void send(T&& elem);

  bool try_send();
  bool try_receive();

  void close(void);
};

template<class T>
class Sudog {
public:
	shared_ptr<condition_variable> cv;
	CaseType isSelect;
	T elem;
	Internal_Channel<T>* c;
    bool done;

    shared_ptr<Sudog> prev;
    shared_ptr<Sudog> next;

    Sudog<T>();
    Sudog<T>(Sudog<T> &sdg);
    static shared_ptr<Sudog<T>> getsudog(void);
};

template<class T>
class SCase {
public:
  CaseType ct;
  shared_ptr<Internal_Channel<T>> c;
  function<void(void)> f;

  SCase(const Channel<T>& c, function<void(void)> f);
  virtual void execute()= 0;
  virtual void recv() = 0;
  virtual void send() = 0;
};

template<class T>
class SCase_Send:public SCase<T> {
public:
  T elem;

  SCase_Send<T>(const Channel<T>& c, T&& elem, function<void(void)> f);
  SCase_Send<T> (const Channel<T>& c, const T& elem, function<void(void)> f);
  void execute();
  void recv();
  void send();
};

template<class T>
class SCase_Recv:public SCase<T> {
public:
  T& elem;

  SCase_Recv<T>(const Channel<T>& c, T &elem, function<void(void)> f);
  void execute();
  void recv();
  void send();
};

template<class T>
class SCase_Def:public SCase<T> {
public:

  SCase_Def<T>(function<void(void)> f);
  void execute();
  void recv();
  void send();
};

template<class T>
class Sudog_List {
public:
  shared_ptr<Sudog<T>> head;
  shared_ptr<Sudog<T>> rear;
  int size;
  
  Sudog_List();
  ~Sudog_List();
  shared_ptr<Sudog<T>> front();
  bool empty();
  void pop_back();
  void pop_front();
  void push_back(const shared_ptr<Sudog<T>>& sg);
};

template<class T>
class Internal_Select {
public:
  deque<shared_ptr<SCase<T>>> cases;
  shared_ptr<SCase_Def<T>> def_case;
  bool running;
  bool defed;
  shared_ptr<condition_variable> cv;
  mutex lock;
  Internal_Select<T>();

  void send(const Channel<T>& c, T&& elem, function<void(void)> f);
  void send(const Channel<T>& c, const T& elem, function<void(void)> f);

  void recv(const Channel<T>& c, T &elem,function<void(void)> f);

  void def(function<void(void)> f);

  void run();
};

template<class T> 
class Select {
public:
    shared_ptr<Internal_Select<T>> ptr;

    Select();

    void send(const Channel<T>& c, T&& elem, function<void(void)> f);
    void send(const Channel<T>& c, const T& elem, function<void(void)> f);

    void recv(const Channel<T>& c, T &elem,function<void(void)> f);

    void def(function<void(void)> f);

    void run();
};

