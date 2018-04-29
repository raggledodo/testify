//
// Created by Mingkai Chen on 2017-04-18.
//
// Implement custom mocker class to test call expectations
//

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <experimental/optional>

#ifndef TESTIFY_MOCKER_HPP
#define TESTIFY_MOCKER_HPP

using namespace std::experimental;

namespace testify
{

struct mocker;

std::string real_str (const mocker* m, std::string key);

struct mocker
{
	mocker (void);

	virtual ~mocker (void);

	mocker (const mocker&);

	mocker (mocker&&);

	mocker& operator = (const mocker&)
	{
		return *this;
	}

	mocker& operator = (mocker&&)
	{
		return *this;
	}

	void label_incr (std::string key) const;

	void set_label (std::string key, std::string value) const;

	static size_t get_usage (mocker* obj, std::string key);

	static optional<std::string> get_value (mocker* obj, std::string key);

	static void clear (void);

private:
	static std::unordered_map<std::string, size_t> usage_;

	static std::unordered_map<std::string, std::string> keyvalue_;

	friend std::string real_str (const mocker* m, std::string key);

	void init (void);

	std::string inst_;
};

}

#endif /* TESTIFY_MOCKER_HPP */

