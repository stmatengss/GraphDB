#ifndef BITMAP_H
#define BITMAP_H

#include <stdlib.h>
#include <omp.h>

#define BITSHRINK(i) (i >> 6)
#define BITOFFSET(i) (i & 0x3f)

typedef struct _Bitmap{
	int _size;
	unsigned long * _data;
	size_t len;

	_Bitmap(int size, int type=0) {	
		this->len = BITSHRINK(size)+1;
		this->_size = size;
		this->_data = new unsigned long [len];

		if (type == 0) this->clear();
		else this->fill();
	
	}

	~_Bitmap() {
		if(this->_data != NULL) {
			free(this->_data);
			this->_data=NULL;
		}
	}

	void fill(){
		#pragma omp parallel for
		for (size_t i=0; i<this->len; i++) {
			this->_data[i] = 0xffffffffffffffff;
		}
		#pragma omp barrier
	}

	void clear(){
		#pragma omp parallel for
		for (size_t i=0; i<this->len; i++) {
			this->_data[i] = 0;
		}
		#pragma omp barrier
	}

	void clear(int start, int count) {
		int end = start + count;
		int s_range = BITSHRINK(start);
		int s_offset= BITOFFSET(start);
		int e_range = BITSHRINK(end);
		int e_offset= BITOFFSET(end);
		if(s_range != e_range) {
			#pragma omp parallel for
			for (int i = s_range+1 ; i < e_range; i++) {
				this->_data[i] = 0;
			}
			#pragma omp barrier
			
			__sync_fetch_and_and(this->_data+s_range, ((1ul<<s_offset) - 1));
			__sync_fetch_and_and(this->_data+e_range, ~((1ul<<e_offset) - 1));
		} else {
			#pragma omp parallel for
			for (int i = start; i < end; i++) {
				clean_bit(i);
			}
			#pragma omp barrier
		}
	}

	void set_bits(_Bitmap * bitmap, int start, int count) {
		int end = start + count;
		int s_range = BITSHRINK(start);
		int s_offset= BITOFFSET(start);
		int e_range = BITSHRINK(end);
		int e_offset= BITOFFSET(end);
		if(s_range != e_range) {
			#pragma omp parallel for
			for (int i = s_range+1 ; i < e_range; i++) {
				this->_data[i] = bitmap->_data[i];
			}
			#pragma omp barrier

			//s_range left
			int s_left = (s_range+1) << 6;
			#pragma omp parallel for
			for (int i = start; i < s_left; i++) {
				if (bitmap->get_bit(i)) set_bit(i);
				else clean_bit(i);
			}
			#pragma omp barrier
			#pragma omp parallel for
			for (int i = e_range << 6; i < end; i++) {
				if (bitmap->get_bit(i)) set_bit(i);
				else clean_bit(i);
			}
			#pragma omp barrier
			//e_range left
			
		} else {
			#pragma omp parallel for
			for (int i = start; i < end; i++) {
				if (bitmap->get_bit(i)) set_bit(i);
				else clean_bit(i);
			}
			#pragma omp barrier
		}
	}

	bool get_bits(int start, int count) {
		int end = start + count;
		int s_range = BITSHRINK(start);
		int s_offset= BITOFFSET(start);
		int e_range = BITSHRINK(end);
		int e_offset= BITOFFSET(end);
		if (s_range != e_range) {
			for (int i = s_range + 1; i < e_range; i++) {
				if (this->_data[i] != 0) { return true; }
			}

			long s = this->_data[s_range];
			long e = this->_data[e_range];
			if ((s & ~((1ul<<s_offset) - 1)) > 0) return true;
			if ((e &  ((1ul<<e_offset) - 1)) > 0) return true; 
		} else {
			for (int i = start; i < end; i ++) {
				if (get_bit(i)) { return true; }
			}
		}
		return false;
	}

	void copy_bits(_Bitmap * b, int start, int count) {
		int end = start + count;
		int s_range = BITSHRINK(start);
		int s_offset= BITOFFSET(start);
		int e_range = BITSHRINK(end);
		int e_offset= BITOFFSET(end);
		int s = s_range, e = e_range - 1;
		if (s_offset == 0) s = s_range;
		else s = s_range + 1;

		#pragma omp parallel for
		for (int i = s; i <= e; i++) {
			this->_data[i] = b->_data[i];
		}
		#pragma omp barrier

		//Set left
		if (s < this->len - 1) {
			int left = (s+1) << 6;
			#pragma omp parallel for
			for (int i = s_offset; i < left; i++) {
				if (b->get_bit(i)) {
					this->set_bit(i);
				} else {
					this->clean_bit(i);
				}
			}
			#pragma omp barrier
		}
		//Set right
		if (e > 0) {
			int right = (e - 1) << 6;
			#pragma omp parallel for
			for (int i = right; i < e_offset; i++) {
				if (b->get_bit(i)) {
					this->set_bit(i);
				} else {
					this->clean_bit(i);
				}
			}
			#pragma omp barrier
		}
	}

	bool get_bit(int i) {
		return bool(this->_data[BITSHRINK(i)] & (1ul << BITOFFSET(i)));
	}

	void set_bit(int i) {
		__sync_fetch_and_or(this->_data+BITSHRINK(i), 1ul<<BITOFFSET(i));
	}

	void clean_bit(int i) {
		__sync_fetch_and_and(this->_data+BITSHRINK(i), ~(1ul<<BITOFFSET(i)));
	}
}Bitmap;

#endif