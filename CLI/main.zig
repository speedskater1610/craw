const std = @import("std");

pub fn main() void {
    const args = std.os.argv;
    
    // args[0] is the program name
    for (args) |arg| {
        std.debug.print("{s}\n", .{arg});
    }
}
