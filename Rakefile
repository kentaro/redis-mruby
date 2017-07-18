VENDOR_DIR    = "vendor"
BUILD_DIR     = "build"
MRUBY_CONFIG  = File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")
MRUBY_PATH    = "#{VENDOR_DIR}/mruby"
REDIS_VERSION = ENV["REDIS_VERSION"] || "stable"
REDIS_PATH    = "#{VENDOR_DIR}/redis-#{REDIS_VERSION}"

directory MRUBY_PATH do
  sh "git clone --depth=1 git://github.com/mruby/mruby.git #{MRUBY_PATH}"
end

file "#{MRUBY_PATH}/build/host/lib/libmruby.a" => MRUBY_PATH do
  sh "cd #{MRUBY_PATH} && rake all MRUBY_CONFIG=#{MRUBY_CONFIG}"
end

directory REDIS_PATH do
  case REDIS_VERSION
  when "stable"
    sh "wget -O - http://download.redis.io/redis-#{REDIS_VERSION}.tar.gz | tar xzvf - -C #{VENDOR_DIR}"
  else
    sh "wget -O - http://download.redis.io/releases/redis-#{REDIS_VERSION}.tar.gz | tar zxvf - -C #{VENDOR_DIR}"
  end
end

desc "compile binary"
task :compile => ["#{MRUBY_PATH}/build/host/lib/libmruby.a", REDIS_PATH] do
  sh "gcc -O2 -shared -fPIC src/mruby.c -I #{MRUBY_PATH}/include -I #{REDIS_PATH}/src -o #{BUILD_DIR}/redis-mruby.so -shared #{MRUBY_PATH}/build/host/lib/libmruby.a"
end

desc "clean up"
task :clean do
  sh "cd #{MRUBY_PATH} && rake deep_clean"
  sh "cd #{REDIS_PATH} && make clean"
end

task :default => :compile