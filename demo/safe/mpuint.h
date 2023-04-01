#ifndef H__MPUINT
#define H__MPUINT

extern void numeric_overflow(void);

class mpuint {
private:
    // ������������
    unsigned short remainder(unsigned short);
    void           shift(unsigned);

public:
    unsigned short *value;
    bool            IsZero(void) const;
    int             Compare(const mpuint &) const;
    int             Compare(unsigned short) const;
    unsigned        length;
    mpuint(unsigned len);
    mpuint(const mpuint &);
    ~mpuint();
    void        operator=(const mpuint &);
    void        operator=(unsigned short);
    void        operator+=(const mpuint &);
    void        operator+=(unsigned short);
    void        operator-=(const mpuint &);
    void        operator-=(unsigned short);
    void        operator*=(const mpuint &);
    void        operator*=(unsigned short);
    void        operator/=(const mpuint &);
    void        operator/=(unsigned short);
    void        operator%=(const mpuint &);
    void        operator%=(unsigned short);
    static void Divide(const mpuint &, const mpuint &, mpuint &, mpuint &);
    char       *edit(char *) const;
    bool        scan(const char *&);
    void        dump() const;
    bool operator==(const mpuint &n) const { return Compare(n) == 0; }
    bool operator!=(const mpuint &n) const { return Compare(n) != 0; }
    bool operator>(const mpuint &n) const { return Compare(n) > 0; }
    bool operator>=(const mpuint &n) const { return Compare(n) >= 0; }
    bool operator<(const mpuint &n) const { return Compare(n) < 0; }
    bool operator<=(const mpuint &n) const { return Compare(n) <= 0; }
    bool operator==(unsigned short n) const { return Compare(n) == 0; }
    bool operator!=(unsigned short n) const { return Compare(n) != 0; }
    bool operator>(unsigned short n) const { return Compare(n) > 0; }
    bool operator>=(unsigned short n) const { return Compare(n) >= 0; }
    bool operator<(unsigned short n) const { return Compare(n) < 0; }
    bool operator<=(unsigned short n) const { return Compare(n) <= 0; }
    static void  Power(const mpuint &, const mpuint &, const mpuint &, mpuint &);
};

#endif
