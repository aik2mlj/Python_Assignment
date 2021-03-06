#include "typetools.h"

MemPool varpool;


//**************************************Bigint definition*************************************

Bigint::Bigint(const string &str) {
    len = str.length();
    if(str[0] == '-') {
        positive = 0;
        --len;
        for(int i = 1; i <= len; i++) num.push_back(str[len - i + 1] - '0');
    } else
        for(int i = 1; i <= len; i++) num.push_back(str[len - i] - '0');
    fit();
}

Bigint::Bigint(const Bigint &that) {
    len = that.len;
    positive = that.positive;
    num.assign(that.num.begin(), that.num.end());
    fit();
}

Bigint::Bigint(long long that) {
    num.clear();
    if(that < 0) positive = 0, that *= -1;
    while(that) {
        num.push_back(that % 10);
        ++len;
        that /= 10;
    }
    fit();
}

Bigint &Bigint::operator=(const string &str) {
    num.clear();
    len = str.length();
    if(str[0] == '-') {
        positive = 0;
        --len;
        for(int i = 1; i <= len; i++) num.push_back(str[len - i + 1] - '0');
    } else
        for(int i = 1; i <= len; i++) num.push_back(str[len - i] - '0');
    fit();
    return *this;
}

Bigint &Bigint::operator=(const Bigint &that) {
    num.clear();
    len = that.len;
    positive = that.positive;
    num.assign(that.num.begin(), that.num.end());
    fit();
    return *this;
}

Bigint &Bigint::operator=(long long that) {
    num.clear();
    if(that < 0) positive = 0, that *= -1;
    while(that) {
        num.push_back(that % 10);
        ++len;
        that /= 10;
    }
    fit();
    return *this;
}

Bigint &Bigint::operator+=(const Bigint &that) { return *this = *this + that; }

Bigint &Bigint::operator-=(const Bigint &that) { return *this = *this - that; }

Bigint &Bigint::operator*=(const Bigint &that) { return *this = *this * that; }

Bigint &Bigint::operator/=(const Bigint &that) { return *this = *this / that; }

Bigint &Bigint::operator%=(const Bigint &that) { return *this = *this % that; }

Bigint &Bigint::operator++() { return *this += 1; }

Bigint Bigint::operator++(int) {
    Bigint res(*this);
    *this += 1;
    return res;
}

Bigint::operator bool() const {
    if(!len || len == 1 && !num[0]) return false;
    else return true;
}

Bigint::operator double() const {
    double res = 0;
    for(int i = len - 1; i >= 0; i--) {
        res *= 10;
        res += num[i];
    }
    return res * (positive? 1: -1);
}

Bigint::operator string() const {
    string str = "";
    for(int i = len - 1; i >= 0; --i) str = str + (char)(num[i] + '0');
    if(str == "") str = "0";
    if(!positive) str = '-' + str;
    return str;
}

void Bigint::fit() {
    while(len > 1 && !num[len - 1]) --len, num.pop_back();
    if(len == 1 && !num[0]) positive = 1;
}

int Bigint::toInt() const {
    int res = 0;
    for(int i = len - 1; i >= 0; i--) {
        res *= 10;
        res += num[i];
    }
    return res * (positive? 1: -1);
}

string Bigint::toString() const {
    string str = "";
    for(int i = len - 1; i >= 0; --i) str = str + (char)(num[i] + '0');
    if(str == "") str = "0";
    if(!positive) str = '-' + str;
    return str;
}

double Bigint::toDouble() const {
    double res = 0;
    for(int i = len - 1; i >= 0; i--) {
        res *= 10;
        res += num[i];
    }
    return res * (positive? 1: -1);
}

void Bigint::print() const {
    if(len == 0) std::cout << "0";
    else {
        if(!positive) std::cout << "-";
        for(int i = len - 1; i >= 0; --i) std::cout << num[i];
    }
    std::cout << std::endl;
}

//----------------------------------operators override-----------------------------------------

Bigint pure_add(const Bigint &ob1, const Bigint &ob2) {
    Bigint res;
    res.positive = ob1.positive;
    res.len = std::max(ob1.len, ob2.len) + 1;
    for(int i = 0; i < res.len; i++) res.num.push_back(0);
    for(int i = 0, g = 0; i < res.len; i++) {
        int tmp = g;
        if(i < ob1.len) tmp += ob1.num[i];
        if(i < ob2.len) tmp += ob2.num[i];
        res.num[i] = tmp % 10;
        g = tmp / 10;
    }
    res.fit();
    return res;
}

Bigint pure_minus(const Bigint &ob1, const Bigint &ob2) {
    Bigint res;
    res.len = ob1.len;
    for(int i = 0, g = 0; i < res.len; i++) {
        int tmp = ob1.num[i] - g;
        if(i < ob2.len) tmp -= ob2.num[i];
        if(tmp >= 0) g = 0;
        else g = 1, tmp += 10;
        res.num.push_back(tmp);
    }
    res.fit();
    return res;
}

Bigint abs(const Bigint &ob) {
    if(ob.positive) return ob;
    else return ob * (-1);
}

int abs_compare(const Bigint &ob1, const Bigint &ob2) {
    if(ob1.len > ob2.len) return 1;
    else if(ob1.len < ob2.len) return -1;
    else {
        for(int i = ob1.len - 1; i >= 0; i--)
            if(ob1.num[i] > ob2.num[i]) return 1;
            else if(ob1.num[i] < ob2.num[i]) return -1;
        return 0;
    }
}

Bigint operator+(const Bigint &ob1, const Bigint &ob2) {
    Bigint res;
    if(ob1.positive) {
        if(ob2.positive) res = pure_add(ob1, ob2);
        else {
            if(abs_compare(ob1, ob2) >= 0) res = pure_minus(ob1, ob2);
            else res = pure_minus(ob2, ob1), res.positive = 0;            
        }
    } else {
        if(!ob2.positive) res = pure_add(ob1, ob2);
        else {
            if(abs_compare(ob2, ob1) >= 0) res = pure_minus(ob2, ob1);
            else res = pure_minus(ob1, ob2), res.positive = 0;
        }
    }
    return res;
}

Bigint operator-(const Bigint &ob1, const Bigint &ob2) {
    return ob1 + (-1) * ob2;
}

Bigint operator*(const Bigint &ob1, const Bigint &ob2) { // +-0!!!
    Bigint res;
    res.positive = !(ob1.positive ^ ob2.positive);
    res.len = ob1.len + ob2.len;
    for(int i = 0; i < res.len; i++) res.num.push_back(0);
    for(int i = 0; i < ob1.len; ++i)
        for(int j = 0; j < ob2.len; ++j) res.num[i + j] += ob1.num[i] * ob2.num[j];
    for(int i = 0; i < res.len - 1; i++) {
        res.num[i + 1] += res.num[i] / 10;
        res.num[i] %= 10;
    }
    res.fit();
    return res;
}

Bigint operator/(const Bigint &ob1, const Bigint &ob2) {
    Bigint res, tmp = 0;
//    if(abs_compare(ob1, ob2) == -1) res = 0;
//    else {
        for(int i = 0; i < ob1.len; ++i) res.num.push_back(0), tmp.num.push_back(0);
        for(int i = ob1.len - 1; i >= 0; --i) {
            tmp = tmp * 10;
            tmp.num[0] = ob1.num[i];
            while(abs_compare(tmp, ob2) >= 0) {
                tmp = tmp - abs(ob2);
                ++res.num[i];
            }
        }
        res.len = ob1.len;
//    }
    res.positive = !(ob1.positive ^ ob2.positive);
    if(!res.positive && tmp) res = res - 1;
    res.fit();
    return res;
}

double floatdivide(const Bigint &ob1, const Bigint &ob2) {
    return ob1.toDouble() / ob2.toDouble();
}

Bigint operator%(const Bigint &ob1, const Bigint &ob2) {
    return ob1 - (ob1 / ob2) * ob2;
}

bool operator==(const Bigint &ob1, const Bigint &ob2) {
    return (ob1.positive == ob2.positive) && !abs_compare(ob1, ob2);
}

bool operator!=(const Bigint &ob1, const Bigint &ob2) {
    return !(ob1 == ob2);
}

bool operator<=(const Bigint &ob1, const Bigint &ob2) {
    return ob1 < ob2 || ob1 == ob2;
}

bool operator>=(const Bigint &ob1, const Bigint &ob2) {
    return ob1 > ob2 || ob1 == ob2;
}

bool operator<(const Bigint &ob1, const Bigint &ob2) {
    if(ob1.positive < ob2.positive) return 1;
    else if(ob1.positive > ob2.positive) return 0;
    else if(ob1.positive) return abs_compare(ob1, ob2) < 0;
    else return abs_compare(ob1, ob2) > 0;
}

bool operator>(const Bigint &ob1, const Bigint &ob2) {
    return !(ob1 <= ob2);
}

//***************************************alltype definition************************************

alltype operator+(const alltype &a, const alltype &b) {
    alltype ans;
    if((a._type == Int || a._type == Bool) && (b._type == Int || b._type == Bool)) {
        ans._type = Int;
        ans.val_int = a.val_int + b.val_int;
    } else if(a._type == Float && (b._type == Int || b._type == Bool)) {
        ans._type = Float;
        ans.val_float = a.val_float + b.val_int.toDouble();
    } else if((a._type == Int || a._type == Bool) && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_int.toDouble() + b.val_float;
    } else if(a._type == Float && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_float + b.val_float;
    } else if(a._type == String && b._type == String) {
        ans._type = String;
        ans.val_string = a.val_string + b.val_string;
    }
    return ans;
}

alltype operator-(const alltype &a, const alltype &b) {
    return a + (-1) * b;
}

alltype operator*(const alltype &a, const alltype &b) {
    alltype ans;
    if((a._type == Int || a._type == Bool) && (b._type == Int || b._type == Bool)) {
        ans._type = Int;
        ans.val_int = a.val_int * b.val_int;
    } else if(a._type == Float && (b._type == Int || b._type == Bool)) {
        ans._type = Float;
        ans.val_float = a.val_float * b.val_int.toDouble();
    } else if((a._type == Int || a._type == Bool) && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_int.toDouble() * b.val_float;
    } else if(a._type == Float && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_float * b.val_float;
    } else if(a._type == String && b._type == Int) {
        ans._type = String;
        int loop_time = b.val_int.toInt();
        while(loop_time--) ans.val_string = ans.val_string + a.val_string;
    } else if(a._type == Int && b._type == String) {
        ans._type = String;
        int loop_time = a.val_int.toInt();
        while(loop_time--) ans.val_string = ans.val_string + b.val_string;
    }
    return ans;
}

alltype operator/(const alltype &a, const alltype &b) {
    alltype ans;
    if((a._type == Int || a._type == Bool) && (b._type == Int || b._type == Bool)) {
        ans._type = Int;
        ans.val_int = floatdivide(a.val_int, b.val_int);
    } else if(a._type == Float && (b._type == Int || b._type == Bool)) {
        ans._type = Float;
        ans.val_float = a.val_float / b.val_int.toDouble();
    } else if((a._type == Int || a._type == Bool) && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_int.toDouble() / b.val_float;
    } else if(a._type == Float && b._type == Float) {
        ans._type = Float;
        ans.val_float = a.val_float / b.val_float;
    }
    return ans;
}

alltype intdivide(const alltype &a, const alltype &b) {
    alltype ans(Int);
    if(a._type == Int && b._type == Int) {
        ans.val_int = a.val_int / b.val_int;
    }
    return ans;
}

alltype operator%(const alltype &a, const alltype &b) {
    alltype ans(Int);
    if(a._type == Int && b._type == Int) {
        ans.val_int = a.val_int % b.val_int;
    }
    return ans;
}

bool operator==(const alltype &a, const alltype &b) {
    return ((a._type == b._type || a._type == Int && b._type == Bool || a._type == Bool && b._type == Int)
            && a.val_int == b.val_int && a.val_float == b.val_float && a.val_string == b.val_string);
}

bool operator!=(const alltype &a, const alltype &b) {
    return !(a == b);
}

bool operator<(const alltype &a, const alltype &b) {
    if((a._type == Int || a._type == Bool) && (b._type == Int || b._type == Bool)) {
        return a.val_int < b.val_int;
    } else if(a._type == Float && (b._type == Int || b._type == Bool)) {
        return a.val_float < b.val_int.toDouble();
    } else if((a._type == Int || a._type == Bool) && b._type == Float) {
        return a.val_int.toDouble() < b.val_float;
    } else if(a._type == Float && b._type == Float) {
        return a.val_float < b.val_float;
    } else if(a._type == String && b._type == String) {
        return a.val_string < b.val_string;
    }
}

bool operator>(const alltype &a, const alltype &b) {
    if((a._type == Int || a._type == Bool) && (b._type == Int || b._type == Bool)) {
        return a.val_int > b.val_int;
    } else if(a._type == Float && (b._type == Int || b._type == Bool)) {
        return a.val_float > b.val_int.toDouble();
    } else if((a._type == Int || a._type == Bool) && b._type == Float) {
        return a.val_int.toDouble() > b.val_float;
    } else if(a._type == Float && b._type == Float) {
        return a.val_float > b.val_float;
    } else if(a._type == String && b._type == String) {
        return a.val_string > b.val_string;
    }
}

bool operator<=(const alltype &a, const alltype &b) { return !(a > b); }

bool operator>=(const alltype &a, const alltype &b) { return !(a < b); }

alltype::alltype() {
    _type = None;
    val_float = 0; val_int = 0; val_string = "";
}

alltype::alltype(Type _, string str = "") {
    _type = _;
    val_int = 0; val_string = ""; val_float = 0;
    switch(_) {
    case None: return;
    case Bool:
        if(str == "True") val_int = 1;
        else val_int = 0;
        return;
    case Int: val_int = str; return;
    case Float: val_float = std::stod(str); return;
    case String: val_string = str; return;
    }
}

alltype::alltype(const alltype &that) { *this = that; }

alltype::alltype(const string that) { *this = that; }

alltype::alltype(int that) { *this = that; }

alltype::alltype(bool that) { *this = that; }

alltype::alltype(double that) { *this = that; }

alltype::alltype(std::pair<string, alltype> that) { *this = that.second; }

alltype &alltype::operator=(Type _) { _type = _; return *this; }

alltype &alltype::operator=(const alltype &that) {
    _type = that._type;
    val_string = that.val_string;
    val_int = that.val_int;
    val_float = that.val_float;
    return *this;
}

alltype &alltype::operator=(const string that) {
    _type = String;
    val_string = that;
    val_int = 0; val_float = 0;
    return *this;
}

alltype &alltype::operator=(int that) {
    _type = Int;
    val_int = that;
    val_float = 0; val_string = "";
    return *this;
}

alltype &alltype::operator=(bool that) {
    _type = Bool;
    val_int = that;
    val_string = ""; val_float = 0;
    return *this;
}

alltype &alltype::operator=(double that) {
    _type = Float;
    val_float = that;
    val_string = ""; val_int = 0;
    return *this;
}

alltype alltype::operator!() {
    return !((bool)(*this));
}

alltype &alltype::operator+=(const alltype &that) { return *this = *this + that; }

alltype &alltype::operator-=(const alltype &that) { return *this = *this - that; }

alltype &alltype::operator*=(const alltype &that) { return *this = *this * that; }

alltype &alltype::operator/=(const alltype &that) { return *this = *this / that; }

alltype &alltype::operator%=(const alltype &that) { return *this = *this % that; }

alltype::operator bool() {
    return (bool)val_int || (bool)val_float || !val_string.empty();
}

string alltype::toString() {
    switch(_type) {
    case None: return "None";
    case Bool:
        if(val_int == 1) return "True";
        else return "False";
    case Int: return val_int.toString();
    case Float: return std::to_string(val_float);
    default: return val_string;
    }
}

//***************************************memall definition****************************************
bool operator<(const memall &a, const memall &b) { return a.data < b.data; }

memall &memall::operator=(const alltype that) {
    if(data == nullptr) data = new alltype;
    *data = that;
    varpool.pool.insert(*this);
    return *this;
}

alltype memall::getData() const { return *data; }

void memall::destruct() { if(data != nullptr) delete data; }