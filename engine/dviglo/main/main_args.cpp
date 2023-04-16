#include "main_args.hpp"

#include <dv_log.hpp>

using namespace std;


namespace dviglo
{

static vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[])
{
    vector<StrUtf8> ret;
    ret.reserve(argc);

    for (i32 i = 0; i < argc; ++i)
        ret.push_back(argv[i]);

    return ret;
}

MainArgs::MainArgs(i32 argc, char* argv[])
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;
    assert(index() == 0); // Подсистема создаётся раньше всех других

    args_ = main_args_to_vector(argc, argv);
}

MainArgs::~MainArgs()
{
    instance_ = nullptr;
}

} // namespace dviglo
