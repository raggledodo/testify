#include <future>
#include <experimental/optional>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "proto/testify.pb.h"
#include "proto/testify.grpc.pb.h"

using namespace std::experimental;

#ifndef CLIENT_HPP
#define CLIENT_HPP

struct ClientConfig
{
	std::string host;
	size_t nretry = 0;
	size_t timeout = 3;
	size_t health_interval = 30;
};

struct DoraClient
{
	DoraClient(ClientConfig configs) :
        configs_(configs), keep_check_(true), reachable_(true)
    {
        stub_ = testify::Dora::NewStub(grpc::CreateChannel(configs.host,
            grpc::InsecureChannelCredentials()));

        health_job_ = new std::thread([this]()
        {
            std::mutex nothing;
            std::unique_lock<std::mutex> lock(nothing);
            while (this->keep_check_.load())
            {
                // check health
                grpc::ClientContext context;
                std::chrono::system_clock::time_point deadline =
                    std::chrono::system_clock::now() +
                    std::chrono::seconds(this->configs_.timeout);
                context.set_deadline(deadline);
                google::protobuf::Empty empty;
                testify::HealthCheckResponse resp;
                auto status = stub_->CheckHealth(&context, empty, &resp);
                auto ecode = status.error_code();
                this->reachable_ = ecode != grpc::StatusCode::DEADLINE_EXCEEDED &&
                    ecode != grpc::StatusCode::UNAVAILABLE &&
                    resp.status() == testify::HealthCheckResponse::SERVING;
                this->stop_check_.wait_for(
                    lock, std::chrono::seconds(this->configs_.health_interval));
            }
        });
    }

    virtual ~DoraClient (void)
    {
        keep_check_ = false;
        stop_check_.notify_all();
        health_job_->join();
        delete health_job_;
    }

    bool reachable (void)
    {
        return reachable_.load();
    }

    // return true if successful
    bool list_testcases (std::unordered_set<std::string>& out)
    {
        if (false == reachable_.load())
        {
            return false;
        }

        grpc::Status status = unsafe_list_testcases(out);

        for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
        {
            // assert: status is not fine, so log
            // mlogger() << status.error_code() << ":" << status.error_message();
            status = unsafe_list_testcases(out);
        }

        bool good = status.ok();
        if (false == good)
        {
            // mlogger() << status.error_code() << ":" << status.error_message();
        }

        return good;
    }

    optional<testify::GeneratedCase> get_one_testcase (std::string tname)
    {
        optional<testify::GeneratedCase> out;
        if (false == reachable_.load())
        {
            return out;
        }

        testify::TransferName request;
        request.set_name(tname);

        testify::GeneratedCase gcase;
        grpc::Status status = unsafe_get_one_testcase(gcase, request);

        for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
        {
            // assert: status is not fine, so log
            // mlogger() << status.error_code() << ":" << status.error_message();
            status = unsafe_get_one_testcase(gcase, request);
        }

        if (status.ok())
        {
            out = gcase;
        }
        else
        {
            // mlogger() << status.error_code() << ":" << status.error_message();
        }

        return out;
    }

    optional<std::vector<testify::GeneratedCase>> get_testcase (
        size_t n, std::string tname)
    {
        optional<std::vector<testify::GeneratedCase>> out;
        if (false == reachable_.load())
        {
            return out;
        }

        testify::TransferName request;
        request.set_name(tname);

        std::vector<testify::GeneratedCase> vec;
        grpc::Status status = unsafe_get_testcase(vec, n, request);

        for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
        {
            // assert: status is not fine, so log
            // mlogger() << status.error_code() << ":" << status.error_message();
            status = unsafe_get_testcase(vec, n, request);
        }

        if (status.ok())
        {
            out = vec;
        }
        else
        {
            // mlogger() << status.error_code() << ":" << status.error_message();
        }

        return out;
    }

	// return true if successful
	bool add_testcase (std::string testname, testify::GeneratedCase& gcase)
	{
        if (false == reachable_.load())
        {
            return false;
        }

		testify::TransferCase tcase;
		tcase.set_name(testname);
		tcase.mutable_results()->Swap(&gcase);

		grpc::Status status = unsafe_add_testcase(tcase);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_add_testcase(tcase);
		}

		bool good = status.ok();
		if (false == good)
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return good;
	}

	// return true if successful
	bool remove_testcase (std::string testname)
	{
        if (false == reachable_.load())
        {
            return false;
        }

		testify::TransferName tname;
		tname.set_name(testname);

		grpc::Status status = unsafe_remove_testcase(tname);

		for (size_t i = 0; i < configs_.nretry && false == status.ok(); ++i)
		{
			// assert: status is not fine, so log
			// mlogger() << status.error_code() << ":" << status.error_message();
			status = unsafe_remove_testcase(tname);
		}

		bool good = status.ok();
		if (false == good)
		{
			// mlogger() << status.error_code() << ":" << status.error_message();
		}

		return good;
	}

	ClientConfig configs_;

private:
	grpc::Status unsafe_list_testcases (std::unordered_set<std::string>& out)
    {
        google::protobuf::Empty request;
        grpc::ClientContext context;
        this->get_context(context);

        grpc::Status status(grpc::UNKNOWN,
            "obtained streaming reader is null");
        auto reader = stub_->ListTestcases(&context, request);
        if (nullptr != reader)
        {
            testify::TransferName tname;
            while (reader->Read(&tname))
            {
                out.emplace(tname.name());
            }
            status = reader->Finish();
        }
        return status;
    }

    grpc::Status unsafe_get_one_testcase (testify::GeneratedCase& gcase,
        testify::TransferName& request)
    {
        grpc::ClientContext context;
        this->get_context(context);

		return stub_->GetOneTestcase(&context, request, &gcase);
    }

    grpc::Status unsafe_get_testcase (std::vector<testify::GeneratedCase>& out,
        size_t nlimit, testify::TransferName& request)
    {
        grpc::ClientContext context;
        this->get_context(context);

        grpc::Status status(grpc::UNKNOWN,
            "obtained streaming reader is null");
        auto reader = stub_->GetTestcase(&context, request);
        if (nullptr != reader)
        {
            testify::GeneratedCase tout;
            for (size_t i = 0; i < nlimit && reader->Read(&tout); ++i)
            {
                out.push_back(tout);
            }
            status = reader->Finish();
        }
        return status;
    }

    grpc::Status unsafe_add_testcase (testify::TransferCase& request)
	{
		google::protobuf::Empty reply;
        grpc::ClientContext context;
        this->get_context(context);

		return stub_->AddTestcase(&context, request, &reply);
	}

    grpc::Status unsafe_remove_testcase (testify::TransferName& request)
	{
		google::protobuf::Empty reply;
        grpc::ClientContext context;
        this->get_context(context);

		return stub_->RemoveTestcase(&context, request, &reply);
	}

    void get_context (grpc::ClientContext& context)
    {
        std::chrono::system_clock::time_point deadline =
            std::chrono::system_clock::now() +
            std::chrono::seconds(configs_.timeout);
        context.set_deadline(deadline);
    }

	std::unique_ptr<testify::Dora::Stub> stub_;
    std::thread* health_job_ = nullptr;

    std::atomic<bool> keep_check_;
    std::atomic<bool> reachable_;
    std::condition_variable stop_check_;
};

#endif /* CLIENT_HPP */
