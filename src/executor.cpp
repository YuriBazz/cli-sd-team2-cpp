#include "executor.hpp"

void Executor::execute(Statement* stmt) {
    if (!stmt) return;
    stmt->execute(env);
}