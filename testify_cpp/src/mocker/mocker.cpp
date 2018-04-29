//
// Created by Mingkai Chen on 2017-04-18.
//

#include "testify_cpp/include/mocker/mocker.hpp"
#include "testify_cpp/include/fuzz/irng.hpp"

#ifdef TESTIFY_MOCKER_HPP

namespace testify
{

const std::string alphanum = "0123456789!@#$%^&*"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static std::unordered_set<std::string> id_set = {""};

std::unordered_map<std::string, size_t> mocker::usage_;
std::unordered_map<std::string, std::string> mocker::keyvalue_;

std::string real_str (const mocker* m, std::string key)
{
	return m->inst_ + "::" + key;
}

mocker::mocker (void)
{
	init();
}

mocker::~mocker (void)
{
	id_set.erase(inst_);
}

mocker::mocker (const mocker&)
{
	init();
}

mocker::mocker (mocker&&)
{
	init();
}

void mocker::label_incr (std::string key) const
{
	std::string realkey = real_str(this, key);
	if (mocker::usage_.end() == mocker::usage_.find(realkey))
	{
		mocker::usage_[realkey] = 0;
	}
	mocker::usage_[realkey]++;
}

void mocker::set_label (std::string key, std::string value) const
{
	std::string realkey = real_str(this, key);
	mocker::keyvalue_[realkey] = value;
}

size_t mocker::get_usage (mocker* obj, std::string key)
{
	return usage_[real_str(obj, key)];
}

optional<std::string> mocker::get_value (mocker* obj, std::string key)
{
	auto it = keyvalue_.find(real_str(obj, key));
	optional<std::string> out;
	if (keyvalue_.end() != it)
	{
		out = it->second;
	}
	return out;
}

void mocker::clear (void)
{
	usage_.clear();
	keyvalue_.clear();
}

void mocker::init (void)
{
	while (id_set.end() != id_set.find(inst_))
	{
		inst_ = std::string(16, ' ');
		std::default_random_engine& gen = get_generator();
		std::uniform_int_distribution<size_t> alphdist(0, alphanum.size());
		for (size_t i = 0; i < 16; ++i)
		{
			inst_[i] = alphanum[alphdist(gen)];
		}
	}
	id_set.emplace(inst_);
}

}

#endif

