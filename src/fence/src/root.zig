const std = @import("std");
const tlds = @import("tlds.zig");
const testing = std.testing;

export fn fence_is_top_level_domain(dom: [*:0]const u8) bool {
    return isTopLevelDomain(std.mem.span(dom));
}

fn isTopLevelDomain(dom: []const u8) bool {
    for (tlds.tab) |tld| {
        if (std.mem.eql(u8, dom, tld)) return true;
    }
    return false;
}

fn isRfc822Char(c: u8) bool {
    if (!std.ascii.isAscii(c)) return false;
    switch (c) {
        0...32, 127 => return false,
        '(', ')', ',', ';', '<', '>', '"' => return false,
    }
    return true;
}

test "check domains" {
    try testing.expect(isTopLevelDomain("com"));
    try testing.expect(isTopLevelDomain("69yamum") == false);

    const str = [_:0]u8{ 'c', 'o', 'm', 0 };
    try testing.expect(fence_is_top_level_domain(str[0..]));
}
