#include <coroutine>
#include <iostream>

struct Generator{
	class ExhaustedValueException : std::exception {};
	struct promise_type {
		// 开始执行时,挂起,等待外部 resume() 获取下一个值
		std::suspend_always initial_suspend() {return {};}

		// 执行结束后,不需要挂起
		std::suspend_always final_suspend() noexcept {return {};}

		void unhandled_exception() {}

		Generator get_return_object()
		{
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		void return_void(){}

		int value;
		bool is_ready = false;

		// await_transform 替换为 yield_value
		std::suspend_always yield_value(int val)
		{
			this->value = val;
			is_ready = true;
			return {};
		}
	};

	std::coroutine_handle<promise_type> handle;

	int next()
	{
		//if (has_next())
		{
			// 此时一定有值, is_ready = true
			// 消费当前的值, 重置 is_ready = false
			handle.promise().is_ready = false;
			return handle.promise().value;
		}

		//throw ExhaustedValueException();
		//handle.resume();

		// 通过 handle 拿到 promise,再拿到 value
		//return handle.promise().value;
	}

	bool has_next()
	{
		// TODO,如果协程已经执行完成,协程的状态已销毁,handle指向的是无效的协程
		// 此时 handle.done() 也是无效的
		// 协程已经执行完成了
		if (handle.done())
		{
			std::cout << "handle is done" << std::endl;
			return false;
		}

		// 协程没执行完成,下一个值没准备好
		if (!handle.promise().is_ready)
		{
			std::cout << "handle not done, not ready, resume" << std::endl;
			handle.resume();
			//return true;
		}

		if (handle.done())
		{
			std::cout << "handle resumed,done" << std::endl;
			// 恢复执行之后,协程执行完
			// 这时候必然没有通过 co_await 传出值来
			return false;
		}
		else
		{
			std::cout << "handle resumed,not done" << std::endl;
			return true;
		}
	}

	~Generator()
	{
		// 销毁协程
		handle.destroy();
	}
};

Generator sequence()
{
	int i = 0;
	while (i < 5)
	{
		co_yield i++;
	}
}

int main()
{
	auto gen = sequence();
	for (int i =0; i < 8; ++i)
	{
		//std::cout << gen.next() << std::endl;
		if (gen.has_next())
		{
			std::cout << gen.next() << std::endl;
		}
		else
		{
			break;
		}
	}
	return 0;
}
