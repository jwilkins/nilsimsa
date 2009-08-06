require 'nilsimsa'

# Levenshtein implementation from
# http://en.wikibooks.org/wiki/Algorithm_implementation/Strings/Levenshtein_distance
# Used under the GNU Free Documentation license
class String
  def levenshtein(other)
    a, b = self.unpack('U*'), other.unpack('U*')
    n, m = a.length, b.length
    a, b, n, m = b, a, m, n if n > m
    current = [*0..n]
    1.upto(m) do |i|
      previous, current = current, [i]+[0]*n
      1.upto(n) do |j|
        add, delete = previous[j]+1, current[j-1]+1
        change = previous[j-1]
        change += 1 if a[j-1] != b[i-1]
        current[j] = [add, delete, change].min
      end
    end
    current[n]
  end
end

n1 = Nilsimsa::new
text1 = "The quick brown fox"
n1.update(text1)
puts "'#{text1}':\n  #{n1.hexdigest}"

n2 = Nilsimsa::new
text2 = "The quick red fox"
n2.update(text2)
puts "'#{text2}':\n  #{n2.hexdigest}"

puts "Distance: #{n1.hexdigest.levenshtein(n2.hexdigest)}"
