#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>

namespace sjtu { 

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = (4096 / sizeof(T)) > 0 ? (4096 / sizeof(T)) : 1;
    T** map;
    size_t map_capacity;
    size_t start_block;
    size_t start_pos;
    size_t sz;

    void reallocate_map(size_t add_front, size_t add_back) {
        size_t num_blocks = sz == 0 ? 0 : (start_pos + sz - 1) / BLOCK_SIZE + 1;
        size_t new_capacity = map_capacity == 0 ? 8 : map_capacity * 2;
        while (new_capacity < num_blocks + add_front + add_back) {
            new_capacity *= 2;
        }
        
        T** new_map = new T*[new_capacity];
        for (size_t i = 0; i < new_capacity; ++i) new_map[i] = nullptr;
        
        size_t new_start_block = (new_capacity - num_blocks) / 2;
        if (add_front > 0 && new_start_block < add_front) {
            new_start_block = add_front;
        }
        
        for (size_t i = 0; i < num_blocks; ++i) {
            new_map[new_start_block + i] = map[start_block + i];
        }
        
        for (size_t i = 0; i < map_capacity; ++i) {
            if (sz == 0 || i < start_block || i >= start_block + num_blocks) {
                if (map[i]) {
                    operator delete(map[i]);
                }
            }
        }
        
        delete[] map;
        map = new_map;
        map_capacity = new_capacity;
        start_block = new_start_block;
    }

public:
	class const_iterator;
	class iterator {
	private:
		deque* dq;
        size_t index;
	public:
        iterator(deque* dq = nullptr, size_t index = 0) : dq(dq), index(index) {}
        
		iterator operator+(const int &n) const {
			return iterator(dq, index + n);
		}
		iterator operator-(const int &n) const {
			return iterator(dq, index - n);
		}
		int operator-(const iterator &rhs) const {
			if (dq != rhs.dq) throw invalid_iterator();
            return index - rhs.index;
		}
		iterator operator+=(const int &n) {
			index += n;
            return *this;
		}
		iterator operator-=(const int &n) {
			index -= n;
            return *this;
		}
		iterator operator++(int) {
            iterator tmp = *this;
            index++;
            return tmp;
        }
		iterator& operator++() {
            index++;
            return *this;
        }
		iterator operator--(int) {
            iterator tmp = *this;
            index--;
            return tmp;
        }
		iterator& operator--() {
            index--;
            return *this;
        }
		T& operator*() const {
            return dq->at(index);
        }
		T* operator->() const noexcept {
            return &(dq->at(index));
        }
		bool operator==(const iterator &rhs) const {
            return dq == rhs.dq && index == rhs.index;
        }
		bool operator==(const const_iterator &rhs) const {
            return dq == rhs.dq && index == rhs.index;
        }
		bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
		bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
        friend class deque;
        friend class const_iterator;
	};
	class const_iterator {
		private:
			const deque* dq;
            size_t index;
		public:
			const_iterator(const deque* dq = nullptr, size_t index = 0) : dq(dq), index(index) {}
			const_iterator(const iterator &other) : dq(other.dq), index(other.index) {}
			const_iterator operator+(const int &n) const {
				return const_iterator(dq, index + n);
			}
			const_iterator operator-(const int &n) const {
				return const_iterator(dq, index - n);
			}
			int operator-(const const_iterator &rhs) const {
				if (dq != rhs.dq) throw invalid_iterator();
                return index - rhs.index;
			}
			const_iterator operator+=(const int &n) {
				index += n;
                return *this;
			}
			const_iterator operator-=(const int &n) {
				index -= n;
                return *this;
			}
			const_iterator operator++(int) {
                const_iterator tmp = *this;
                index++;
                return tmp;
            }
			const_iterator& operator++() {
                index++;
                return *this;
            }
			const_iterator operator--(int) {
                const_iterator tmp = *this;
                index--;
                return tmp;
            }
			const_iterator& operator--() {
                index--;
                return *this;
            }
			const T& operator*() const {
                return dq->at(index);
            }
			const T* operator->() const noexcept {
                return &(dq->at(index));
            }
			bool operator==(const iterator &rhs) const {
                return dq == rhs.dq && index == rhs.index;
            }
			bool operator==(const const_iterator &rhs) const {
                return dq == rhs.dq && index == rhs.index;
            }
			bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }
			bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }
            friend class deque;
            friend class iterator;
	};

	deque() : map(nullptr), map_capacity(0), start_block(0), start_pos(0), sz(0) {}
	deque(const deque &other) {
        map_capacity = other.map_capacity;
        start_block = other.start_block;
        start_pos = other.start_pos;
        sz = other.sz;
        if (map_capacity > 0) {
            map = new T*[map_capacity];
            for (size_t i = 0; i < map_capacity; ++i) map[i] = nullptr;
        } else {
            map = nullptr;
        }
        
        if (sz > 0) {
            size_t num_blocks = (start_pos + sz - 1) / BLOCK_SIZE + 1;
            for (size_t i = 0; i < num_blocks; ++i) {
                map[start_block + i] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
            }
            size_t constructed = 0;
            try {
                for (size_t i = 0; i < sz; ++i) {
                    size_t block_offset = (start_pos + i) / BLOCK_SIZE;
                    size_t pos_in_block = (start_pos + i) % BLOCK_SIZE;
                    new (map[start_block + block_offset] + pos_in_block) T(other.map[start_block + block_offset][pos_in_block]);
                    constructed++;
                }
            } catch (...) {
                for (size_t i = 0; i < constructed; ++i) {
                    size_t block_offset = (start_pos + i) / BLOCK_SIZE;
                    size_t pos_in_block = (start_pos + i) % BLOCK_SIZE;
                    map[start_block + block_offset][pos_in_block].~T();
                }
                for (size_t i = 0; i < map_capacity; ++i) {
                    if (map[i]) operator delete(map[i]);
                }
                delete[] map;
                throw;
            }
        }
    }
	~deque() {
        clear();
        for (size_t i = 0; i < map_capacity; ++i) {
            if (map[i]) {
                operator delete(map[i]);
            }
        }
        delete[] map;
    }
	deque &operator=(const deque &other) {
        if (this == &other) return *this;
        clear();
        for (size_t i = 0; i < map_capacity; ++i) {
            if (map[i]) operator delete(map[i]);
        }
        delete[] map;
        
        map_capacity = other.map_capacity;
        start_block = other.start_block;
        start_pos = other.start_pos;
        sz = other.sz;
        if (map_capacity > 0) {
            map = new T*[map_capacity];
            for (size_t i = 0; i < map_capacity; ++i) map[i] = nullptr;
        } else {
            map = nullptr;
        }
        
        if (sz > 0) {
            size_t num_blocks = (start_pos + sz - 1) / BLOCK_SIZE + 1;
            for (size_t i = 0; i < num_blocks; ++i) {
                map[start_block + i] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
            }
            size_t constructed = 0;
            try {
                for (size_t i = 0; i < sz; ++i) {
                    size_t block_offset = (start_pos + i) / BLOCK_SIZE;
                    size_t pos_in_block = (start_pos + i) % BLOCK_SIZE;
                    new (map[start_block + block_offset] + pos_in_block) T(other.map[start_block + block_offset][pos_in_block]);
                    constructed++;
                }
            } catch (...) {
                for (size_t i = 0; i < constructed; ++i) {
                    size_t block_offset = (start_pos + i) / BLOCK_SIZE;
                    size_t pos_in_block = (start_pos + i) % BLOCK_SIZE;
                    map[start_block + block_offset][pos_in_block].~T();
                }
                for (size_t i = 0; i < map_capacity; ++i) {
                    if (map[i]) operator delete(map[i]);
                }
                delete[] map;
                map = nullptr;
                map_capacity = 0;
                sz = 0;
                throw;
            }
        }
        return *this;
    }
	T & at(const size_t &pos) {
        if (pos >= sz) throw index_out_of_bound();
        size_t block_offset = (start_pos + pos) / BLOCK_SIZE;
        size_t pos_in_block = (start_pos + pos) % BLOCK_SIZE;
        return map[start_block + block_offset][pos_in_block];
    }
	const T & at(const size_t &pos) const {
        if (pos >= sz) throw index_out_of_bound();
        size_t block_offset = (start_pos + pos) / BLOCK_SIZE;
        size_t pos_in_block = (start_pos + pos) % BLOCK_SIZE;
        return map[start_block + block_offset][pos_in_block];
    }
	T & operator[](const size_t &pos) {
        return at(pos);
    }
	const T & operator[](const size_t &pos) const {
        return at(pos);
    }
	const T & front() const {
        if (sz == 0) throw container_is_empty();
        return at(0);
    }
	const T & back() const {
        if (sz == 0) throw container_is_empty();
        return at(sz - 1);
    }
	iterator begin() { return iterator(this, 0); }
	const_iterator cbegin() const { return const_iterator(this, 0); }
	iterator end() { return iterator(this, sz); }
	const_iterator cend() const { return const_iterator(this, sz); }
	bool empty() const { return sz == 0; }
	size_t size() const { return sz; }
	void clear() {
        for (size_t i = 0; i < sz; ++i) {
            size_t block_offset = (start_pos + i) / BLOCK_SIZE;
            size_t pos_in_block = (start_pos + i) % BLOCK_SIZE;
            map[start_block + block_offset][pos_in_block].~T();
        }
        sz = 0;
        start_block = map_capacity / 2;
        start_pos = 0;
    }
	iterator insert(iterator pos, const T &value) {
        if (pos.dq != this || pos.index > sz) throw invalid_iterator();
        size_t idx = pos.index;
        if (idx == 0) {
            push_front(value);
            return iterator(this, 0);
        }
        if (idx == sz) {
            push_back(value);
            return iterator(this, sz - 1);
        }
        
        if (idx < sz / 2) {
            push_front(front());
            for (size_t i = 1; i < idx; ++i) {
                (*this)[i] = (*this)[i + 1];
            }
            (*this)[idx] = value;
        } else {
            push_back(back());
            for (size_t i = sz - 2; i > idx; --i) {
                (*this)[i] = (*this)[i - 1];
            }
            (*this)[idx] = value;
        }
        return iterator(this, idx);
    }
	iterator erase(iterator pos) {
        if (pos.dq != this || pos.index >= sz) throw invalid_iterator();
        size_t idx = pos.index;
        if (idx == 0) {
            pop_front();
            return iterator(this, 0);
        }
        if (idx == sz - 1) {
            pop_back();
            return iterator(this, sz);
        }
        
        if (idx < sz / 2) {
            for (size_t i = idx; i > 0; --i) {
                (*this)[i] = (*this)[i - 1];
            }
            pop_front();
            return iterator(this, idx);
        } else {
            for (size_t i = idx; i < sz - 1; ++i) {
                (*this)[i] = (*this)[i + 1];
            }
            pop_back();
            return iterator(this, idx);
        }
    }
	void push_back(const T &value) {
        if (sz == 0) {
            if (map_capacity == 0) reallocate_map(0, 1);
            start_block = map_capacity / 2;
            start_pos = 0;
            if (!map[start_block]) map[start_block] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
            new (map[start_block] + start_pos) T(value);
            sz++;
            return;
        }
        size_t block_offset = (start_pos + sz) / BLOCK_SIZE;
        size_t pos_in_block = (start_pos + sz) % BLOCK_SIZE;
        size_t actual_block = start_block + block_offset;
        
        if (actual_block >= map_capacity) {
            reallocate_map(0, 1);
            actual_block = start_block + block_offset;
        }
        
        if (!map[actual_block]) {
            map[actual_block] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
        }
        
        new (map[actual_block] + pos_in_block) T(value);
        sz++;
    }
	void pop_back() {
        if (sz == 0) throw container_is_empty();
        size_t block_offset = (start_pos + sz - 1) / BLOCK_SIZE;
        size_t pos_in_block = (start_pos + sz - 1) % BLOCK_SIZE;
        size_t actual_block = start_block + block_offset;
        
        map[actual_block][pos_in_block].~T();
        sz--;
    }
	void push_front(const T &value) {
        if (sz == 0) {
            if (map_capacity == 0) reallocate_map(1, 0);
            start_block = map_capacity / 2;
            start_pos = BLOCK_SIZE - 1;
            if (!map[start_block]) map[start_block] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
            new (map[start_block] + start_pos) T(value);
            sz++;
            return;
        }
        
        if (start_pos == 0) {
            if (start_block == 0) {
                reallocate_map(1, 0);
            }
            start_block--;
            start_pos = BLOCK_SIZE - 1;
        } else {
            start_pos--;
        }
        
        if (!map[start_block]) {
            map[start_block] = reinterpret_cast<T*>(operator new(BLOCK_SIZE * sizeof(T)));
        }
        
        try {
            new (map[start_block] + start_pos) T(value);
        } catch (...) {
            if (start_pos == BLOCK_SIZE - 1) {
                start_block++;
                start_pos = 0;
            } else {
                start_pos++;
            }
            throw;
        }
        sz++;
    }
	void pop_front() {
        if (sz == 0) throw container_is_empty();
        map[start_block][start_pos].~T();
        if (start_pos == BLOCK_SIZE - 1) {
            start_block++;
            start_pos = 0;
        } else {
            start_pos++;
        }
        sz--;
    }
};

}

#endif