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
    assert(index() == 0); // Подсистема создаётся раньше всех других

    args_ = main_args_to_vector(argc, argv);

    instance_ = this;

    // Это сообщение попадёт только в консоль, так как лог ещё не открыт
    Log::write_debug("MainArgs constructed");
}

MainArgs::~MainArgs()
{
}

} // namespace dviglo
