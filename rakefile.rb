COMPILE = "gcc -c --std=c99 -Iconf -O3 -o"
LINK = "gcc -O3 -o"
WITH_DEPS = "-lm $(pkg-config gstreamer-1.0 --cflags --libs)"
WITH_CONF = "-DCONFIG="

configs = Dir["conf/*.h"].map {|p| File.basename p, ".*"}
modules = Dir["src/*.c"].map {|p| File.basename p, ".*"}

configs.each do |c|
  obj_paths = modules.map {|m| "obj/#{c}_#{m}.o"}
  modules.each do |m|
    file "obj/#{c}_#{m}.o" => ["src/#{m}.c", "src/app.h", "conf/#{c}.h"] do
      sh "#{COMPILE} obj/#{c}_#{m}.o src/#{m}.c #{WITH_CONF}#{c} #{WITH_DEPS}"
    end
  end
  file "bin/#{c}" => obj_paths do
    sh "#{LINK} bin/#{c} #{obj_paths.join " "} #{WITH_DEPS}"
  end
  task "build_#{c}" => "bin/#{c}"
  task "run_#{c}" => "build_#{c}" do
    sh "bin/#{c}"
  end
end

task :default => :build
task :build => :build_default
task :run => :run_default
task :clean do; sh "rm -rf obj/* bin/*"; end
