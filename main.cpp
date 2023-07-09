#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>

#include <cstdlib>
#include <ctime>

using namespace std;
using namespace chrono;

class Item {
	string value_;
	Item *prev_, *next_;
public:

	const string& value() const { return value_;  }
	Item* prev() { return prev_;  }
	void set_prev(Item* _prev) { prev_ = _prev; }
	Item* next() { return next_; }
	void set_next(Item* _next) { next_ = _next; }

	Item(const string& _value, Item *_prev = 0, Item *_next = 0)
		: value_(_value), prev_(_prev), next_(_next)
	{		
	}
};

const string empty_string = "";

class Bucket {
	uint64_t size_;
	Bucket *first_half_bucket, *second_half_bucket;
	Item* first_item_;

public:

	Item* first_item() { return first_item_;  }
	uint64_t size() { return size_;  }

	Bucket(Item *_first_item, Bucket *_first_half_bucket = 0, Bucket *_second_half_bucket = 0, uint64_t _size = 1)
		: first_item_(_first_item), size_(_size), first_half_bucket(_first_half_bucket), second_half_bucket(_second_half_bucket)
	{
	}

	Bucket *insert(const string& _str)
	{
		if (size_ == 1)
		{			
			// place before
			if (_str <= first_item_->value())
			{
			
				auto prev = first_item_->prev() ? first_item_->prev() : 0;
				auto item = new Item(_str, prev, first_item_);

				if (first_item_->prev())
				{
					first_item_->prev()->set_next(item);
				}

				first_item_->set_prev(item);
				
				
				auto bucket = new Bucket(item, new Bucket(item), this, 2);
				return bucket;
			}

			// place after
			auto next = first_item_->next() ? first_item_->next() : 0;
			auto item = new Item(_str, first_item_, next);

			if (first_item_->next())
			{
				first_item_->next()->set_prev(item);
			}

			first_item_->set_next(item);

			auto bucket = new Bucket(first_item_, this, new Bucket(item), 2);
			return bucket;
		}

		size_++;

		if (_str <= second_half_bucket->first_item_->value())
		{
			first_half_bucket = first_half_bucket->insert(_str);
			first_item_ = first_half_bucket->first_item_;
		}
		else
		{
			second_half_bucket = second_half_bucket->insert(_str);
		}

		return this;
	}

	// _index offset from first
	const string& get(uint64_t _index)
	{
		if (_index >= size_)
		{
			return empty_string;
		}

		// plain search
		if (_index < 7)
		{
			auto item = first_item_;
			while (_index)
			{
				item = item->next();
				_index--;
			}
			return item->value();
		}

		// search in second
		if (_index >= first_half_bucket->size_)
		{
			return second_half_bucket->get(_index - first_half_bucket->size_);
		}

		// take in first
		return first_half_bucket->get(_index);
	}

	Bucket *erase(uint64_t _index)
	{
		if (_index >= size_)
		{
			return this;
		}

		if (size_ == 1)
		{
			// relink

			if (first_item_->prev()) first_item_->prev()->set_next(first_item_->next());
			if (first_item_->next()) first_item_->next()->set_prev(first_item_->prev());

			delete first_item_;
			return 0;
		}

		size_--;
		// search in second
		if (_index >= first_half_bucket->size_)
		{			
			auto r = second_half_bucket->erase(_index - first_half_bucket->size_);
			if (r == 0)
			{
				delete second_half_bucket;
				return first_half_bucket;
			}

			if (second_half_bucket != r)
			{
				delete second_half_bucket;
			}

			second_half_bucket = r;
			return this;			
		}

		// erase in first
		auto r = first_half_bucket->erase(_index);

		if (r == 0)
		{
			delete first_half_bucket;
			first_item_ = second_half_bucket->first_item_;
			return second_half_bucket;
		}

		if (first_half_bucket != r)
		{
			delete first_half_bucket;
			first_half_bucket = r;
		}
		
		if (_index == 0)
		{
			first_item_ = first_half_bucket->first_item_;
		}

		return this;
	}

	void debug(string level = "")
	{

		cout << level << "size: " << size_ << ", value: '" << first_item_->value() + "'\n";
		if (first_half_bucket) {
			cout << level << "first:\n";
			first_half_bucket->debug(level + " ");
		}
		if (second_half_bucket) {
			cout << level << "second:\n";
			second_half_bucket->debug(level + " ");
		}
	}

};

class storage {
public:
	Bucket *head;
	storage() {
		head = 0;
	}

	void balance()
	{
		// for current data not needed 

		return;

		/*
		if (!head) return;
		if (!head->first_half_bucket || !head->second_half_bucket) return;
		if (abs((int64_t)head->first_half_bucket->size - (int64_t)head->second_half_bucket->size) > 1) {
			if (head->first_half_bucket->size > head->second_half_bucket->size) {

			}
			else {

			}
		}
		*/
	}

	void insert(const string& _str)
	{
		if (!head)
		{
			head = new Bucket(new Item(_str));
			return;
		}

		head = head->insert(_str);
		// balance();
	}
	void erase(uint64_t _index)
	{
		if (head)
		{
			auto r = head->erase(_index);

			if (r == 0)
			{
				delete head;
				head = 0;
			}
			else
			{
				if (r != head)
				{
					delete head;
					head = r;
				}
				// balance();
			}
		}
	}
	const string& get(uint64_t _index)
	{
		if (!head)
		{
			return empty_string;
		}

		return head->get(_index);
	}

	void debug() {
		if (head) {

			auto item = head->first_item();
			int idx = 0;
			while (item) {
				cout << idx << ": '" << item->value() << "'\n";

				idx++;
				item = item->next();
			}

			head->debug();
		}
	}
};

using write_sequence = vector<string>;

using test_pair = pair<uint64_t, string>;
using modify_sequence = vector<test_pair>;
using read_sequence = vector<test_pair>;

ifstream& operator >> (ifstream& _is, test_pair& _value)
{
	_is >> _value.first;
	_is >> _value.second;

	return _is;
}

template <typename S>
S get_sequence(const string& _file_name)
{
	ifstream infile(_file_name);
	S result;

	typename S::value_type item;

	while (infile >> item)
	{
		result.emplace_back(move(item));
	}

	return result;
}


int main() {

	// string path = "c:/t/t2/";
	string path = "";

	write_sequence write = get_sequence<write_sequence>(path + "write.txt");
	modify_sequence modify = get_sequence<modify_sequence>(path + "modify.txt");
	read_sequence read = get_sequence<read_sequence>(path + "read.txt");

	storage st;

	for (const string& item : write)
	{
		st.insert(item);
	}

	uint64_t progress = 0;
	uint64_t percent = modify.size() / 100;

	time_point<system_clock> time;
	nanoseconds total_time(0);

	modify_sequence::const_iterator mitr = modify.begin();
	read_sequence::const_iterator ritr = read.begin();

	for (; mitr != modify.end() && ritr != read.end(); ++mitr, ++ritr)
	{
		time = system_clock::now();
		st.erase(mitr->first);
		st.insert(mitr->second);
		const string& str = st.get(ritr->first);
		total_time += system_clock::now() - time;

		if (ritr->second != str)
		{
			cout << "test failed " << ritr->first << ", " << ritr->second << " : " << str << endl;
			return 1;
		}

		
		if (++progress % (5 * percent) == 0)
		{
			cout << "time: " << duration_cast<milliseconds>(total_time).count()
				<< "ms progress: " << progress << " / " << modify.size() << "\n";
		}			
	}

	cout << "time: " << duration_cast<milliseconds>(total_time).count() << "\n";
};