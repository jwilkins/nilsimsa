require 'bundler'
Bundler.require :test

$LOAD_PATH.unshift File.expand_path("../lib", __FILE__)
require 'nilsimsa'
#require "#{File.join(File.dirname(__FILE__), '..', 'ext', 'nilsimsa_native')}"

describe Nilsimsa do

  it "new() then update() should compute hexdigests for short strings correctly" do
    n1 = Nilsimsa::new
    n1.update("abcdefgh")
    #puts "abcdefgh:  #{n1.hexdigest=='14c8118000000000030800000004042004189020001308014088003280000078'}"
    n1.hexdigest.should == '14c8118000000000030800000004042004189020001308014088003280000078'
  end

  it "new() should compute hexdigests correctly for an array of strings" do
    n1 = Nilsimsa::new
    n1.update("abcdefgh")
    n2 = Nilsimsa::new("abcd","efgh")
    #puts "abcd efgh: #{n2.hexdigest=='14c8118000000000030800000004042004189020001308014088003280000078'}"
    #puts "digest:    #{n1 == n2.digest}"
    n2.hexdigest.should =='14c8118000000000030800000004042004189020001308014088003280000078'
    n1.should == n2.digest
  end

  it "should allow updates" do
    n1 = Nilsimsa::new("abcdefgh")
    n1.hexdigest.should == '14c8118000000000030800000004042004189020001308014088003280000078'
    n1.update("ijk")
    n1.hexdigest.should == '14c811840010000c0328200108040630041890200217582d4098103280000078'
    n2 = Nilsimsa::new("abcd","efgh")
    n1.nilsimsa(n2.digest).should == 109
  end

#    #puts 'Running selftest using native ruby version'
#    #selftest
#    begin                               # load C core - if available
#      if File.exists?('./nilsimsa_native')
#        require './nilsimsa_native'
#        puts 'Running selftest using compiled nilsimsa in current dir'
#      elsif File.exists?('./ext/nilsimsa_native')
#        require './ext/nilsimsa_native'
#        puts 'Running selftest using compiled nilsimsa in current dir'
#      else 
#        require 'nilsimsa_native'
#        puts 'Running selftest using compiled nilsimsa'
#      end
#      selftest
#    rescue LoadError => e
#      puts "Couldnt run selftest with compiled nilsimsa"
#    end

end
