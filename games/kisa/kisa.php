�#!/usr/bin/php
<?
/* Set internal character encoding to UTF-8 */
mb_internal_encoding ("UTF-8");

include "phrases.php";

$ввод = fopen ('php://stdin', 'r');

print "Киса офлайн.\n";
print "Привет! Познакомимся?\n";
for (;;) {
	print '>> ';
	$запрос = fgets ($ввод);
	if (! $запрос) {
		print "\n";
		break;
	}
	$запрос = trim ($запрос);
	$длина_запроса = mb_strlen ($запрос);
print ("Длина = $длина_запроса\n");
/*
	$вопрос = 0;
	if ($длина_запроса < 2) {
		$настроение = 1;
		qz[1] := qz[1] + 1;
	} else if ($запрос == $предыдущий_запрос)
		$настроение = 2;
	} else {
		while (o) and (a) do begin
			z := input[k];
			if z = '?' then
				$вопрос = 1;
			k := k+1;
			if z = input[k] then
				n := n+1;
			if n > 4 then
				o := false;
			if k = input_length then
				a := false
		end;
	}
	lastmess := input;
	if not(o) then begin
		qz[1] := qz[1] + 1;
		$настроение := 1
	end
*/

# 	parse;
	$вывод = "Вижу: <$запрос>";
	print ($вывод . "\n");
}
?>
