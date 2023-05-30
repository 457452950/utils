#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

#include "wutils/buffer/ChainBuffer.h"
#include "wutils/buffer/RingBuffer.h"
#include "wutils/buffer/StraightBuffer.h"

inline void test_straightbuffer() {
    using namespace std;
    using namespace wutils;

    char        temp[65536]{0};
    std::string str_temp;

    auto buffer = std::unique_ptr<Buffer>(new StraightBuffer);
    if(!buffer->Init(UINT16_MAX)) {
        cout << "init false. " << strerror(errno) << endl;
        return;
    }
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->UpdateWriteBytes(65535U);
    assert(buffer->IsFull() == true);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65535U);
    assert(buffer->GetWriteableBytes() == 0);
    assert(buffer->UsedBytes() == 65535U);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->UpdateWriteBytes(65534U);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65534U);
    assert(buffer->GetWriteableBytes() == 1);
    assert(buffer->UsedBytes() == 65534U);
    assert(buffer->MaxSize() == UINT16_MAX);
    assert(buffer->ReadFixBytes((uint8_t *)temp, 65535U) == false);
    assert(buffer->WriteFixBytes((uint8_t *)"abc", 3) == false);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    uint64_t total = 0;
    for(int i = 0; i < 150; ++i) {
        //              "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera"
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        assert(buffer->WriteFixBytes((uint8_t *)s, 73));
        total += 73;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);
        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = buffer->Write((uint8_t *)s, 73);

        total += len;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    int         i = 0;
    const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

    //    buffer->WriteUntil([s, &total, &i](uint8_t *buffer, uint64_t buffer_len) -> uint64_t {
    //        if(i == 15000) {
    //            return 0;
    //        }
    //        auto len = std::min(buffer_len, 73ULL);
    //        memcpy(buffer, s, len);
    //        total += len;
    //        ++i;
    //        return len;
    //    });
    assert(total == buffer->UsedBytes());
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;


    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);
    total = 0;

    for(int i = 0; i < 15000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        assert(buffer->WriteFixBytes((uint8_t *)s, 73));
        total += 73;
        assert(total == buffer->UsedBytes());

        assert(buffer->ReadFixBytes((uint8_t *)temp, 73));
        total -= 73;

        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
        assert(string(s, 73) == string(temp, 73));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);

        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        assert(total == buffer->UsedBytes());

        memcpy(temp, buffer->PeekRead(), len);
        buffer->SkipReadBytes(len);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

        auto len = buffer->Write((uint8_t *)s, 73);
        total += len;
        assert(total == buffer->UsedBytes());

        len = buffer->Read((uint8_t *)temp, 73);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
};

inline void test_ringbuffer() {
    using namespace std;
    using namespace wutils;

    char        temp[65536]{0};
    std::string str_temp;

    auto buffer = std::unique_ptr<Buffer>(new RingBuffer);
    if(!buffer->Init(UINT16_MAX)) {
        cout << "init false. " << strerror(errno) << endl;
        return;
    }
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->UpdateWriteBytes(65535U);
    assert(buffer->IsFull() == true);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65535U);
    assert(buffer->GetWriteableBytes() == 0);
    assert(buffer->UsedBytes() == 65535U);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    buffer->UpdateWriteBytes(65534U);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65534U);
    assert(buffer->GetWriteableBytes() == 1);
    assert(buffer->UsedBytes() == 65534U);
    assert(buffer->MaxSize() == UINT16_MAX);
    assert(buffer->ReadFixBytes((uint8_t *)temp, 65535U) == false);
    assert(buffer->WriteFixBytes((uint8_t *)"abc", 3) == false);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);

    uint64_t total = 0;
    for(int i = 0; i < 150; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        assert(buffer->WriteFixBytes((uint8_t *)s, 73));
        total += 73;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    for(int i = 0; i < 150; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);
        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    for(int i = 0; i < 150000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = buffer->Write((uint8_t *)s, 73);

        total += len;
        //        cout << "t " << total << " u " << buffer->UsedBytes() << endl;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    int         i = 0;
    const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

    buffer->WriteUntil([s, &total, &i](uint8_t *buffer, uint64_t buffer_len) -> uint64_t {
        if(i == 15000) {
            return 0;
        }
        auto len = std::min<uint64_t>(buffer_len, 73ULL);
        memcpy(buffer, s, len);
        total += len;
        ++i;
        return len;
    });
    assert(total == buffer->UsedBytes());
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;


    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == UINT16_MAX);
    total = 0;

    for(int i = 0; i < 150000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        assert(buffer->WriteFixBytes((uint8_t *)s, 73));
        total += 73;
        assert(total == buffer->UsedBytes());

        assert(buffer->ReadFixBytes((uint8_t *)temp, 73));
        total -= 73;
        assert(total == buffer->UsedBytes());
        assert(string(s, 73) == string(temp, 73));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);

        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        assert(total == buffer->UsedBytes());

        memcpy(temp, buffer->PeekRead(), len);
        buffer->SkipReadBytes(len);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

        auto len = buffer->Write((uint8_t *)s, 73);
        total += len;
        assert(total == buffer->UsedBytes());

        len = buffer->Read((uint8_t *)temp, 73);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
};


inline void test_chainbuffer() {
    using namespace std;
    using namespace wutils;

    char        temp[65536]{0};
    std::string str_temp;

    auto buffer = std::unique_ptr<Buffer>(new ChainBuffer);
    if(!buffer->Init(UINT16_MAX)) {
        cout << "init false. " << strerror(errno) << endl;
        return;
    }
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);

    buffer->UpdateWriteBytes(65535U);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65535U);
    assert(buffer->GetWriteableBytes() == 65535U);
    assert(buffer->UsedBytes() == 65535U);
    assert(buffer->MaxSize() == 4194240UL);

    buffer->SkipReadBytes(4194240UL);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);

    buffer->UpdateWriteBytes(65534U);
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == false);
    assert(buffer->GetReadableBytes() == 65534U);
    assert(buffer->GetWriteableBytes() == 1);
    assert(buffer->UsedBytes() == 65534U);
    assert(buffer->MaxSize() == 4194240UL);
    assert(buffer->ReadFixBytes((uint8_t *)temp, 65535U) == false);
    assert(buffer->WriteFixBytes((uint8_t *)"abc", 3) == true);

    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);

    uint64_t total = 0;
    for(int i = 0; i < 15000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        if(buffer->WriteFixBytes((uint8_t *)s, 73))
            total += 73;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    for(int i = 0; i < 15000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);
        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    for(int i = 0; i < 15000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = buffer->Write((uint8_t *)s, 73);

        total += len;
        assert(total == buffer->UsedBytes());
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
    int         i = 0;
    const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

    buffer->WriteUntil([s, &total, &i](uint8_t *buffer, uint64_t buffer_len) -> uint64_t {
        if(i == 15000) {
            return 0;
        }
        auto len = std::min<uint64_t>(buffer_len, 73ULL);
        memcpy(buffer, s, len);
        total += len;
        ++i;
        return len;
    });
    assert(total == buffer->UsedBytes());
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;


    buffer->SkipAllReadBytes();
    assert(buffer->IsFull() == false);
    assert(buffer->IsEmpty() == true);
    assert(buffer->GetReadableBytes() == 0);
    assert(buffer->GetWriteableBytes() == UINT16_MAX);
    assert(buffer->UsedBytes() == 0);
    assert(buffer->MaxSize() == 4194240UL);
    total = 0;

    for(int i = 0; i < 150000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        assert(buffer->WriteFixBytes((uint8_t *)s, 73));
        total += 73;
        assert(total == buffer->UsedBytes());

        assert(buffer->ReadFixBytes((uint8_t *)temp, 73));
        total -= 73;
        assert(total == buffer->UsedBytes());
        assert(string(s, 73) == string(temp, 73));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s   = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";
        auto        len = std::min<uint64_t>(buffer->GetWriteableBytes(), 73ULL);

        memcpy(buffer->PeekWrite(), s, len);
        buffer->UpdateWriteBytes(len);
        total += len;
        assert(total == buffer->UsedBytes());

        memcpy(temp, buffer->PeekRead(), len);
        buffer->SkipReadBytes(len);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;

    for(int i = 0; i < 150000; ++i) {
        const char *s = "asdsafsagaesgeragregaregreagasdsafsagaesgeragregaregreagasdsafsagaesgera";

        auto len = buffer->Write((uint8_t *)s, 73);
        total += len;
        assert(total == buffer->UsedBytes());

        len = buffer->Read((uint8_t *)temp, 73);
        total -= len;
        assert(total == buffer->UsedBytes());
        assert(string(s, len) == string(temp, len));
    }
    cout << "t " << total << " u " << buffer->UsedBytes() << endl;
};

inline void test_buffer() {
    test_straightbuffer();
    test_ringbuffer();
    test_chainbuffer();
}

int main() {
    test_buffer();
    return 0;
}
