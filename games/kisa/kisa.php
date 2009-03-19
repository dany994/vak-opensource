#!/usr/bin/php
<?
/* Устанавливаем внутреннюю кодировку UTF-8 */
mb_internal_encoding ("UTF-8");

include "phrases.php";

/*
 * Ищем в строке последовательности одинаковых букв.
 * Возвращаем максимальную длину такой последовательности.
 */
function одинаковые_буквы ($строка, $длина)
{
	$max = 0;
	$n = 0;
	$last_c = '';
	for ($i=0; $i<$длина; ++$i) {
		$c = mb_substr ($строка, $i, 1);
//print ("<$i:$c>");
		if ($c == $last_c) {
//print ("<$c==$last_c>");
			++$n;
			if ($n > $max)
				$max = $n;
		} else {
			$n = 0;
			$last_c = $c;
		}
	}
//if ($max > 1) print ("<$max одинаковых букв>\n");
	return $max;
}

function обиделись ()
{
	global $фразы;

	print ($фразы [0] [2]);
	print ("\n");
	exit ();
}

function разбор ($запрос)
{
//print ("разбор ($запрос)\n");
	/* Есть ли вопросительный знак. */
	$вопрос = (mb_strpos ($запрос, "?") >= 0);

	if (mb_ereg_match ('.*как.*меня.*зовут', $запрос)) {
//print ("разбор ($запрос) вернул 3\n");
		return 3;
	}
	if (mb_stristr ($запрос, 'ты') && mb_stristr ($запрос, 'кто')) {
//print ("разбор ($запрос) вернул 4\n");
		return 4;
	}
/*	if (reply_needed) and ((poisk(':)',true)) or (poisk(':-)',true))) {
		reply_needed := false;
		$настроение := 5;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((poisk(':(',true)) or (poisk(':-(',true))) {
		reply_needed := false;
		$настроение := 6;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((poisk('да?',true)) or (poisk('да',true)) and
	    (poisk('?',false))) {
		reply_needed := false;
		$настроение := 7;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((poisk('да',true)) or (poisk('да.',true)) or
	    (poisk('да',true)) and (poisk('!',false))) {
		reply_needed := false;
		$настроение := 8;
		vyvod := poslevyvod
	}
*/
/*	if (reply_needed) and ((poisk('нет',true)) or (poisk('нет.',true)) or
	    (poisk('нет',true)) and (poisk('!',false))) {
		reply_needed := false;
		$настроение := 9;
		vyvod := poslevyvod
	}
	if (reply_needed) and (poisk('привет',true)) {
		reply_needed := false;
		$настроение := 10;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((poisk('здорова',true)) or
	    (poisk('здорово',true))) {
		reply_needed := false;
		$настроение := 11;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((input='давай') or (input='давай.') or (input='давай!')) {
		reply_needed := false;
		$настроение := 12;
		vyvod := poslevyvod
	}
*/
/*	if (reply_needed) and (((poisk('как',true)) and (poisk('дела',false))) or
	    ((poisk('как',true)) and (poisk('жизнь',false))) or
	    ((poisk('как',true)) and (poisk('твое',false)) and
	    (poisk('ничего',false)))) {
		reply_needed := false;
		$настроение := 13;
		vyvod := poslevyvod
	}
	if (reply_needed) and (poisk('как',true)) and (poisk('поживаеш',true)) {
		reply_needed := false;
		$настроение := 14;
		vyvod := poslevyvod
	}
	if (reply_needed) and ((input='пока') or (input='прощай') or (input='до свидания') or
	    (input='до скорого') or (input='бай')) {
		randomize;
		reply_needed := false;
		$настроение := 15;
		qz[15] := random(6) + 112;
		writeln (poslevyvod);
		halt;
	}
	if (reply_needed) and ((poisk('почему',true)) or ((poisk('почему',true)) and
	    (poisk('?',false)))) {
		reply_needed := false;
		$настроение := 16;
		vyvod := poslevyvod
	}
	if (reply_needed) and (question) {
		reply_needed := false;
		$настроение := 17;
		vyvod := poslevyvod
	}
	if (reply_needed) and (poisk('дура',true)) {
		reply_needed := false;
		$настроение := 18;
		vyvod := poslevyvod
	}*/
/*	if (reply_needed) and ((poisk('блядь',true)) or (poisk('блять',true)) or
	    (poisk('гандон',true)) or (poisk('дура',true)) or
	    (poisk('ебану',true)) or (poisk('ебать',true)) or
	    (pos('ебись',input)>-1) or (pos('ебля',input)>-1) or
	    (pos('ебну',input)>-1) or (pos('пизда',input)>-1) or
	    (pos('проститутка',input)>-1) or (pos('уебище',input)>-1) or
	    (pos('хуйня',input)>-1) or (pos('шлюха',input)>-1)) {
		reply_needed := false;
		$настроение := 19;
		vyvod := poslevyvod
	} */
/*	if reply_needed {
		reply_needed := false;
		$настроение := 20;
		vyvod := poslevyvod
	}
	if reply_needed
		if poisk('сколько',true) and poisk('будет:',false)
			vyvod := 'С данным вопросом обращайся к другим программам'
		else if poisk('сколько',true) and poisk('будет',false)
			vyvod := 'С данным вопросом обращайся к другим программам'
		else if poisk('посчитай:',true)
			vyvod := 'С данным вопросом обращайся к другим программам'
		else if poisk('посчитай',true)
			vyvod := 'С данным вопросом обращайся к другим программам';
*/
}

$ввод = fopen ('php://stdin', 'r');
$память = array ();
$память [0] = 1;

print "Киса офлайн.\n";
print "Привет! Познакомимся?\n";
for (;;) {
	print '>> ';
	$запрос = fgets ($ввод);
	if (! $запрос) {
		print "\n";
		break;
	}
	$запрос = mb_strtolower (trim ($запрос));
	$длина_запроса = mb_strlen ($запрос);
//print ("Длина = $длина_запроса\n");

	if ($длина_запроса < 2 || одинаковые_буквы ($запрос, $длина_запроса) > 5) {
//print ("<Короткий или странный запрос>\n");
		$настроение = 1;
	} else if ($запрос == $предыдущий_запрос) {
//print ("<Повторяющийся запрос>\n");
		$настроение = 2;
	} else
		$настроение = разбор ($запрос);
	$предыдущий_запрос = $запрос;

	if (++$память [$настроение] > count ($фразы [$настроение])) {
		if ($настроение == 1 || $настроение == 2 || $настроение == 19) {
			if ($память [$настроение] == count ($фразы [$настроение]) + 1) {
				$настроение = 0;
			} else
				обиделись ();
		} else
			$память [$настроение] = 1;
	}
	$вывод = $фразы [$настроение] [$память [$настроение]];

	print ($вывод);
	print ("\n");
}
?>
