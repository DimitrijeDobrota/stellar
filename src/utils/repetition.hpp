#ifndef STELLAR_REPETITION_H
#define STELLAR_REPETITION_H

namespace repetition {

class Table {
  public:
    Table() = default;

    bool is_repetition(const U64 hash) const {
        for (int i = repetitions.size() - 1; i >= 0; i--) {
            if (repetitions[i] == hash) return true;
            if (repetitions[i] == hashNull) return false;
        }
        return false;
    }

    void pop(void) { repetitions.pop_back(); }
    void clear(void) { repetitions.clear(); }
    void push_null(void) { repetitions.push_back(hashNull); }
    void push_hash(U64 hash) { repetitions.push_back(hash); }

    friend std::ostream &operator<<(std::ostream &os, const Table &rtable) {
        for (const U64 hash : rtable.repetitions)
            os << hash << " ";
        return os;
    }

  private:
    std::vector<U64> repetitions;

    const static int hashNull = 0;
};

} // namespace repetition

#endif
