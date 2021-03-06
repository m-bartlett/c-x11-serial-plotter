#include <stdio.h>
#include <cstring>

#define DEFAULT_RING_SIZE 40

struct Ring {
	int *data;
	size_t size;
	size_t rindex=0;
	int mindex=0;
	int maxdex=0;

	Ring() { 
		data = new int[DEFAULT_RING_SIZE];
		this->size = DEFAULT_RING_SIZE;
		memset(this->data, 0, this->size);
	};
	Ring(int sz) {
		data = new int[sz];
		this->size = sz;
		memset(this->data, 0, this->size);
	};
	~Ring() { delete []data; }
	int data_min() { return this->data[mindex]; }
	int data_max() { return this->data[maxdex]; }

	void insert(int v) {
		this->data[rindex] = v;
		if (rindex == mindex) {
			int min = 999999;
			for (int i=0; i < this->size; i++) {
				if (min > this->data[i]) {
					mindex = i;
					min = this->data[i];
				}
			}
		}
		if (rindex == maxdex) {
			int max = 0;
			for (int i=0; i < this->size; i++) {
				if (max < this->data[i]) {
					maxdex = i;
					max = this->data[i];
				}
			}
		}
		if (v < this->data[mindex]) mindex = rindex;
		else if (v > this->data[maxdex]) maxdex = rindex;
		rindex = (++rindex) >= this->size ? 0 : rindex;
	}

	void memcpy(int *buff) { // Assumes buff is the same size as ring.size
		int j=0;
		for (int i=this->rindex; i < this->size; i++, j++) buff[j] = this->data[i];
		for (int i=0; i < this->rindex; i++, j++) buff[j] = this->data[i];
	}

	void get_normalized_buffer(int *buff, float scalar, int min, int max) {
		if (!scalar || max <= min) {memset(buff, 0, this->size); return; }
		const float normalized_scalor = scalar / (max - min);
		int i, j=0;
		for (i=this->rindex; i < this->size; i++, j++) buff[j] = int((this->data[i]-min) * normalized_scalor);
		for (i=0; i < this->rindex; i++, j++) buff[j] = int((this->data[i]-min) * normalized_scalor);
	}

	void get_auto_normalized_buffer(int *buff, float scalar) {
		int min = this->data[mindex];
		int max = this->data[maxdex];
		get_normalized_buffer(buff, scalar, min, max);
	}

	void get_static_min_normalized_buffer(int *buff, float scalar, int min) {
		int max = this->data[maxdex];
		get_normalized_buffer(buff, scalar, min, max);
	}

	void get_static_max_normalized_buffer(int *buff, float scalar, int max) {
		int min = this->data[mindex];
		get_normalized_buffer(buff, scalar, min, max);
	}

	void get_static_normalized_buffer(int *buff, float scalar, int min, int max) {
		get_normalized_buffer(buff, scalar, min, max);
	}

	void print() {
		for (int i=this->rindex; i < this->size;   i++) printf("%d ", this->data[i]);
		for (int i=0;            i < this->rindex; i++) printf("%d ", this->data[i]);
	}

	void zero() { memset(this->data, 0, this->size); }

};