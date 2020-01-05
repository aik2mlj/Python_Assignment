#ifndef PYTHON_INTERPRETER_TYPETOOLS_H
#define PYTHON_INTERPRETER_TYPETOOLS_H

#include <string>
#include <vector>
#include <set>
#include <iostream>
using std::string;

class Bigint {
    friend Bigint pure_add(const Bigint &, const Bigint &);
    friend Bigint pure_minus(const Bigint &, const Bigint &);
    friend Bigint abs(const Bigint &);
    friend int abs_compare(const Bigint &, const Bigint &); // 1: >; 0: ==; -1: <

    friend Bigint operator+(const Bigint &, const Bigint &);
    friend Bigint operator-(const Bigint &, const Bigint &);
    friend Bigint operator*(const Bigint &, const Bigint &);
    friend Bigint operator/(const Bigint &, const Bigint &); // 整除
    friend double floatdivide(const Bigint &, const Bigint &); // 实数除
    friend Bigint operator%(const Bigint &, const Bigint &);
    friend bool operator==(const Bigint &, const Bigint &);
    friend bool operator!=(const Bigint &, const Bigint &);
    friend bool operator<=(const Bigint &, const Bigint &);
    friend bool operator>=(const Bigint &, const Bigint &);
    friend bool operator<(const Bigint &, const Bigint &);
    friend bool operator>(const Bigint &, const Bigint &);

private:
    bool positive = 1;
    int len = 0;
    std::vector<int> num;
    void fit();

public:
    Bigint() { num.clear(); len = 0; positive = 1; }
    Bigint(const string &);
    Bigint(const Bigint &);
    // Bigint(const Bigint &&);
    Bigint(long long);
    Bigint &operator=(const string &);
    Bigint &operator=(long long);
    Bigint &operator=(const Bigint &);
    Bigint &operator+=(const Bigint &);
    Bigint &operator-=(const Bigint &);
    Bigint &operator*=(const Bigint &);
    Bigint &operator/=(const Bigint &);
    Bigint &operator%=(const Bigint &);
    Bigint &operator++();
    Bigint operator++(int);
    explicit operator bool() const;
    explicit operator double() const;
    explicit operator string() const;
    int toInt() const;
    string toString() const;
    double toDouble() const;
    void print() const;
    ~Bigint() {}
};

// -------------------------------alltype-----------------------------------
enum Type { None, Bool, Int, Float, String };
enum Ftype { Normal, Continue, Break, Return };
enum Comp_Op { L, M, E, LE, ME, NE };
enum Arithmetic { Add, Minus, Times, Divide, IDivide, Mode };

class alltype {
    friend alltype operator+(const alltype &, const alltype &);
    friend alltype operator-(const alltype &, const alltype &);
    friend alltype operator*(const alltype &, const alltype &);
    friend alltype operator/(const alltype &, const alltype &); //实数除
    friend alltype intdivide(const alltype &, const alltype &);
    friend alltype operator%(const alltype &, const alltype &);
    friend bool operator==(const alltype &, const alltype &);
    friend bool operator!=(const alltype &, const alltype &);
    friend bool operator<=(const alltype &, const alltype &);
    friend bool operator>=(const alltype &, const alltype &);
    friend bool operator<(const alltype &, const alltype &);
    friend bool operator>(const alltype &, const alltype &);

private:

public:
    Type _type;
    // bool is stored in val_int, too
    Bigint val_int;
    double val_float;
    string val_string;

    alltype();
    alltype(Type, string);
    alltype(const alltype &);
    alltype(const string);
    alltype(int);
    alltype(bool);
    alltype(double);
    explicit alltype(std::pair<string, alltype>);
    alltype &operator=(Type);
    alltype &operator=(const alltype &);
    alltype &operator=(const string);
    alltype &operator=(int);
    alltype &operator=(bool);
    alltype &operator=(double);
    alltype operator!();
    alltype &operator+=(const alltype &);
    alltype &operator-=(const alltype &);
    alltype &operator*=(const alltype &);
    alltype &operator/=(const alltype &);
    alltype &operator%=(const alltype &);
    explicit operator bool();
    string toString();
};

typedef std::pair<string, alltype> Var;

class flowtype {
public:
    Ftype _type;
    std::vector<alltype> val;
    flowtype(Ftype _): _type(_) { val.clear(); }
    flowtype(Ftype _, std::vector<alltype> v): _type(_), val(v) {}
};

class memall {
    friend bool operator<(const memall &, const memall &);
private:
    alltype* data = nullptr;
public:
    alltype getData() const;
    memall &operator=(const alltype);
    void destruct();
};


typedef std::set<memall> VarSet;
class MemPool {
public:
    VarSet pool;
    MemPool() { pool.clear(); }
    ~MemPool() {
        for(auto it = pool.begin(); it != pool.end(); ++it) {
            auto now = *it;
            now.destruct();
        }
    }
};

#endif // PYTHON_INTERPRETER_TYPETOOLS_H