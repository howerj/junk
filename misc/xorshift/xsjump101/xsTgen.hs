import Data.List
import Text.Printf

data Expr =
  Zero
  | Var String Integer
  | Xor [Expr]
  deriving (Eq, Show)

bus name cntr = map (Var name) [0 .. cntr - 1]

zeroes = Zero : zeroes

xor x y = zipWith (\x y -> Xor [x, y]) x y

crop x y = zipWith (\ x y -> x) x y

shl x n = crop (take n zeroes ++ x) x

shr x n = crop (drop n x ++ zeroes) x

flatten::Expr -> [Expr]
flatten (Xor (x:xs)) = flatten x ++ (concatMap flatten xs)
flatten (Xor [])   = []
flatten x          = [x]

matrix e vars = zipWith (\e v -> (v, makeRow e)) e vars where
  makeRow e = map (\v -> length (filter (== v) e) `mod` 2) vars 

packMatrix m vars = map (\(v, b) -> (v, tail (packRow b vars "" 0))) m where
  packRow xb@(b:bs) xv@((Var n i):vs) vname acc | n == vname = packRow bs vs vname (acc + b*2^i)
                                                | otherwise  = acc : packRow xb xv n 0               
  packRow [] _ _ acc = [acc]
  packRow _ [] _ acc = [acc]

matrixCode pm = header ++ (concat $ intersperse ",\n" (map (\(v, r) -> "  {" ++ hexRow r ++ "}") pm)) ++ "\n};\n" where
  header = printf "#include <cstdint>\n\nuint32_t T[%d][%d] = {\n" (length pm) (length $ snd $ head pm)
  hexRow r = concat $ intersperse ", " (map (printf "0x%08XU") r)

main =
  do
    putStr (matrixCode $ packMatrix (matrix (map flatten xs128) vars) vars)
  where x = bus "x" 32 
        y = bus "y" 32
        z = bus "z" 32
        w = bus "w" 32
        vars = (x++y++z++w)
        t = x `xor` (x `shl` 11)
        xs128 = y ++ z ++ w ++ (w `xor` (w `shr` 19) `xor` t `xor` (t `shr` 8))

