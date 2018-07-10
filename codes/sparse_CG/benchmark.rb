#!/usr/bin/env ruby
# vim: se sw=2 ts=2 et:
require 'fileutils'
require 'yaml'
require 'cgi'

def ignore_warnings
  old_stderr = $stderr
  $stderr = StringIO.new
  yield
ensure
  $stderr = old_stderr
end

require 'rubygems'
begin
  ignore_warnings {
    require 'gruff'
  }
rescue LoadError
  puts "require 'gruff' failed. Trying to install it..."

  ENV['PKG_CONFIG_PATH'] = '/usr/lib64/pkgconfig'
  system('gem install --user-install gruff')

  puts 'gruff installed. Please try again!'
  exit 1
end

class Benchmark
  ALL_OPTIONS = File.read('CMakeLists.txt').scan(/option\((.*?)\s/i).flatten
  ROOT_DIR = Dir.pwd

public
  def initialize(config)
    yaml = YAML::load(File.read(config))
    @build_dir = ((yaml['config'] or {})['build_dir'] or "#{ROOT_DIR}/build")
    @build_dir.gsub!(/\$ROOT_DIR/, ROOT_DIR)
    @targets = yaml['targets']
    @flags = {}

    @targets.each { |name, target|
      @flags[name] = ''

      ALL_OPTIONS.each { |opt|
        @flags[name] += " -D#{opt}=" +
                        if target['options'].include?(opt)
                          'ON'
                        else
                          'OFF'
                        end
      }

      @flags[name] += " #{target['cmake_flags']}"
    }

    if File.exists?(@build_dir)
      puts "'#{@build_dir}' already exists!"
      exit 1
    end
  end

  def build(name)
    build_dir = "#{@build_dir}/#{name}"
    log = "#{@build_dir}/#{name}_build.log"

    FileUtils.mkdir_p(build_dir)

    # Just a fallback, please set it in your ENV!
    unless ENV.has_key?('LIS_INCLUDE_DIR')
      ENV['LIS_INCLUDE_DIR'] = "#{ENV['HOME']}/.local/include"
    end
    unless ENV.has_key?('LIS_LIBRARY_DIR')
      ENV['LIS_LIBRARY_DIR'] = "#{ENV['HOME']}/.local/lib"
    end

    env_cmd = "module load cmake >#{log} 2>&1; " +
              "module load #{@targets[name]['compiler']} >#{log} 2>&1"
    cmd_prefix = if @targets[name].has_key?('host')
                   "ssh #{@targets[name]['host']}"
                 else
                   'zsh -c'
                 end

    puts "Configuring target #{name}"
    if not system("#{cmd_prefix} '#{env_cmd}; cd #{build_dir}; " +
                  "cmake #{@flags[name]} #{ROOT_DIR} >>#{log} 2>&1'")
      puts "Failed to configure for #{name}!"
      return
    end

    puts "Building target #{name}"
    if not system("#{cmd_prefix} '#{env_cmd}; cd #{build_dir}; " +
                  "make >>#{log} 2>&1'")
      puts "Failed to make for #{name}!"
      return
    end
  end

  def run(name)
    target = @targets[name]

    return if !target.has_key?('tests')
    return if !target.has_key?('matrix')

    matrix = target['matrix']

    target['tests'].each { |test|
      log = "#{@build_dir}/#{name}_#{test.gsub(/\//, '_')}_run.log"
      xvec = "#{@build_dir}/#{name}_#{test.gsub(/\//, '_')}_run.xvec"

      env_cmd = "module load #{@targets[name]['compiler']} >#{log} 2>&1"
      env = (@targets[name]['environment'] or []).join(' ')
      wrapper = @targets[name]['wrapper'] or ''

      cmd = if target.has_key?('host')
              "ssh #{target['host']} '#{env_cmd} && cd #{ROOT_DIR} && " +
              "time CG_OUTPUT_FILE=\"#{xvec}\" #{env} #{wrapper} " +
              "#{@build_dir}/#{name}/#{test} #{matrix}" + "' >>\"#{log}\" 2>&1"
            else
              "zsh -c '#{env_cmd} && cd #{ROOT_DIR} && " +
              "time CG_OUTPUT_FILE=\"#{xvec}\" #{env} #{wrapper} " +
              "#{@build_dir}/#{name}/#{test} #{matrix}" + "' >>\"#{log}\" 2>&1"
            end

      puts "Running target #{name} test #{test}"
      if not system(cmd)
        puts "Failed to run test #{test} for #{name}!"
        puts "Command: #{cmd}"
      end
    }
  end

  def check(name)
    target = @targets[name]

    return if !target.has_key?('tests')
    return if !target.has_key?('matrix')

    target['tests'].each { |test|
      log = File.read("#{@build_dir}/#{name}_#{test.gsub(/\//, '_')}_run.log")
      match = log.match(/RESULT CHECK:\s*(.*)/)

      unless match
        puts "Check failed for #{name} test #{test} (result check missing)"
        next
      end
      if match[1] != 'OK'
        puts "Check failed for #{name} test #{test} (result check != ok)"
        next
      end

      puts "Check ok for #{name} test #{test}"
    }
  end

  def build_all
    @targets.each { |name, _| build(name) }
  end

  def run_all
    @targets.each { |name, _| run(name) }
  end

  def check_all
    @targets.each { |name, _| check(name) }
  end

  def output_all
    FileUtils.mkdir_p("#{@build_dir}/html")
    FileUtils.mkdir_p("#{@build_dir}/csv")

    html = File.open("#{@build_dir}/html/index.html", 'w')
    html.write('<html><body>')

    time_csv = File.open("#{@build_dir}/csv/time.csv", 'w')
    time_csv.write("Test")

    gflops_csv = File.open("#{@build_dir}/csv/gflops.csv", 'w')
    gflops_csv.write("Test")

    time_data = {}
    gflops_data = {}

    @targets.each { |name, target|
      next if !target.has_key?('tests')
      next if !target.has_key?('matrix')

      time_graph = Gruff::Bar.new
      time_graph.minimum_value = 0
      time_graph.maximum_value = 0

      gflops_graph = Gruff::Bar.new
      gflops_graph.minimum_value = 0
      gflops_graph.maximum_value = 0

      time_csv.write(";#{name}")
      gflops_csv.write(";#{name}")

      target['tests'].each { |test|
        log = File.read("#{@build_dir}/#{name}_#{test.gsub(/\//, '_')}_run.log")

        time = log.match(/Total time:\s*(.*)/)
        gflops = log.match(/MatVec GFLOP\/s:\s(.*)/)

        test_basename = File.basename(test)

        if time
          time_graph.data(test_basename, time[1].to_f)
        end
        if gflops
          gflops_graph.data(test_basename, gflops[1].to_f)
        end

        time_data[test_basename] = {} if !time_data.has_key?(test_basename)
        gflops_data[test_basename] = {} if !gflops_data.has_key?(test_basename)

        time_data[test_basename][name] = if time
                                           time[1].strip
                                         else
                                           ""
                                         end
        gflops_data[test_basename][name] = if gflops
                                             gflops[1].strip
                                           else
                                             ""
                                           end
      }

      time_graph.write("#{@build_dir}/html/#{name}_time.png")
      gflops_graph.write("#{@build_dir}/html/#{name}_gflops.png")

      cfg = target.to_yaml
      cfg.gsub!(/^---\s*\n/, '')

      name = CGI.escapeHTML(name)
      cfg = CGI.escapeHTML(cfg)

      html.write <<-eos
        <h2>#{name}</h2>
        <table border='0' cellpadding='5'>
         <tr>
          <td valign='top'>
           Total time:<br/>
           <img src='#{name}_time.png' alt='#{name}_time.png'/>
          </td>
          <td rowspan='2' valign='top'>
           <pre>#{cfg}</pre>
           <p>
            <a href='../#{name}_build.log'>Build log</a>
           </p>
          </td>
         </tr>
         <tr>
          <td valign='top'>
           GFLOP/s:<br/>
           <img src='#{name}_gflops.png' alt='#{name}_gflops.png'>
          </td>
         </tr>
        </table>
      eos
    }

    html.write('</body></html>')

    time_csv.write("\n")
    gflops_csv.write("\n")

    def write_csv(csv, data)
      data.each { |name, test|
        csv.write(name)

        @targets.each { |targetname, _|
          csv.write(";#{data[name][targetname]}")
        }

        csv.write("\n")
      }
    end

    write_csv(time_csv, time_data)
    write_csv(gflops_csv, gflops_data)

    time_csv.close
    gflops_csv.close
  end
end

if ARGV.count != 1
  puts "Syntax: #{$0} benchmark.yaml"
  exit 1
end

benchmark = Benchmark.new(ARGV[0])
benchmark.build_all
benchmark.run_all
benchmark.check_all
benchmark.output_all
