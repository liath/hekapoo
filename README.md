### Hekapoo
So a couple years ago I started porting [samyk/opensesame](https://github.com/samyk/opensesame) to the Si4010.
I think all of this worked but I got distracted with other projects and haven't
gotten around to my eventual goal of making this into a badge shaped like the
those really cool scissors from `Star vs. the Forces of Evil`.

### Building
Place the Si4010 examples/common folders contents into firmware/common and copy
over the demo's keyfob_startup.a51 to firmware/hekapoo/src/keyfob_startup.a51.
You can download them here:
https://www.silabs.com/documents/public/example-code/Si4010_example_programs.zip

You'll also need the Keil C51 compiler, free version is available here:
https://www.keil.com/demo/eval/c51.htm

Then just run build.sh. IIRC, you'll need some other stuff to actually run this
on the Si4010, but I can't remember. /shrug
