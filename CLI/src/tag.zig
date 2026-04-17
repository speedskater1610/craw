const std = @import("std");

fn printHelp() !void {
    const stdout = std.io.getStdOut().writer();
    try stdout.print("\t`help`\t-\tPrints help (This message!)\n", .{});
    try stdout.print("\t`build`\t-\tBuilds the craw project in the current directory\n", .{});
    try stdout.print("\t`run`\t-\tRuns the built project, if the project isnt built it will build it\n",  .{});
    try stdout.print("\t`new`\t-\tWill prompt for name and then setup the project there\n", .{});

    try stdout.flush();
}

fn setupWorkspace() void {
    const stdout = std.io.getStdOut().writer();
    const stdin = std.io.getStdIn().reader();

    try stdout.print("dir: ", .{});

    var buf: [100]u8 = undefined;

    if (try stdin.readUntilDelimiterOrEof(&buf, '\n')) |user_input| {
        const cleaned_input = std.mem.trimRight(u8, user_input, "\r");

        try stdout.print("Using dir: {s}!\n", .{cleaned_input});
    }
}

pub fn getTag(tag: []const u8) !void {
    if (std.mem.eql(u8, tag, "help")) {
        printHelp();
    }
    else if (std.mem.eql(u8, tag, "build")) {
        //  TODO: impl build logic
    }
    else if (std.mem.eql(u8, tag, "run")) {
        // TODO: impl build logic
    }
    else if (std.mem.eql(u8, tag, "new")) {
        setupWorkspace();
    }
}