#pragma once
#ifndef UTIL_INSTANCEHELPER_H
#define UTIL_INSTANCEHELPER_H

#define Instance(CLASS)                                                                                                \
public:                                                                                                                \
    static CLASS *GetInstance() {}                                                                                     \
                                                                                                                       \
private:                                                                                                               \
    static CLASS *instance_;                                                                                           \
    CLASS(const CLASS &)            = delete;                                                                          \
    CLASS(CLASS &&)                 = delete;                                                                          \
    CLASS &operator=(const CLASS &) = delete;


#define InstanceInit(VALUE, ...) __VA_ARGS__ *__VA_ARGS__ ::instance_ = VALUE;

#endif // UTIL_INSTANCEHELPER_H
