#pragma once

namespace nvl {

/**
 * @enum WalkResult
 * @brief Describes how to continue an interruptible, recursive preorder traversal after visiting a node.
 */
enum class WalkResult {
    kRecurse,   // Continue a recursive walk
    kNoRecurse, // Continue the walk, but do not recurse through children of the currently visited node
    kExit       // Terminate the walk after the currently visited node
};

} // namespace nvl
