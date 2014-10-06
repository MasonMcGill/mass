COMPILE = "gcc -c --std=c99 -Iconf -O3 -o"
LINK = "gcc -O3 -o"
DEPS = "-lm " + `pkg-config gstreamer-1.0 --cflags --libs`
CONF = "-DCONFIG="
DELETE = "rm -f"

configs = Dir["conf/*.h"].map {|p| File.basename p, ".*"}
modules = Dir["src/*.c"].map {|p| File.basename p, ".*"}

configs.each do |c|
  obj_paths = modules.map {|m| "obj/#{c}_#{m}.o"}
  modules.each do |m|
    file "obj/#{c}_#{m}.o" => ["src/#{m}.c", "src/app.h", "conf/#{c}.h"] do
      sh *"#{COMPILE}obj/#{c}_#{m}.o src/#{m}.c #{CONF}#{c} #{DEPS}".split
    end
  end
  file "bin/#{c}" => obj_paths do
    sh *"#{LINK} bin/#{c} #{obj_paths.join " "} #{DEPS}".split
  end
  task "build_#{c}" => "bin/#{c}"
  task "run_#{c}" => "build_#{c}" do
    sh *"bin/#{c}".split
  end
end

task :default => :build
task :build => :build_default
task :run => :run_default
task :clean do; sh "#{DELETE} obj/* bin/*"; end
