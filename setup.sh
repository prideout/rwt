alias init="cmake -H. -Bbuild"
alias initdbg="cmake -DCMAKE_BUILD_TYPE=Debug -H. -Bbuild"
alias build="cmake --build build"
alias clean="rm -rf build"
alias debug="lldb ./build/kopf -o run"
alias run="./build/kopf"
