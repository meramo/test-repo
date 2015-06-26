<?php

function activehelper_livehelp__monthly_chats() {
  global $activehelper_livehelp;

  $output = '<h2>Monthly chats</h2>';

  drupal_add_css('div.form-item-from { float: left; margin-right: 25px; }', 'inline');

  // Define the default values for the table sort
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Agent');
  }
  if (!isset($_GET['sort'])) {
    $_GET['sort'] = 'asc';
  }
  if (!isset($_GET['page'])) {
    $_GET['page'] = 0;
  }

  $form = drupal_get_form('activehelper_livehelp__monthly_chats_date_form');
  $output .= render($form);

  switch ($_GET['order']) {
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Chats'):
      $order_field = "COUNT( jls.id )";
      break;
    case t('Agent'):
    default:
      $order_field = "CONCAT( jlu.firstname, ' ', jlu.lastname )";
  }
  $order_type = $_GET['sort'];

  $limit_from = $_GET['page'] * $activehelper_livehelp['listing_limit'];
  $limit_to = ( $_GET['page'] + 1 ) * $activehelper_livehelp['listing_limit'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare table header
  $table_header = array(
    array('data' => t('Agent'), 'field' => 'agent_fullname'),
    array('data' => t('Domain name'), 'field' => 'domain_name'),
    array('data' => t('Chats'), 'field' => 'chats'),
  );

  $query = "
		SELECT
			CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_fullname,
			jld.name AS domain_name,
			COUNT( jls.id ) AS chats
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jls.datetime, '%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jls.datetime, '%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jls.id_user = jlu.id
			AND jls.id_domain = jld.id_domain
		GROUP BY
			jls.id_domain, jlu.username, CONCAT( jlu.firstname, ' ', jlu.lastname )
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  $limit_max = db_query("
		SELECT COUNT( result.chats )
		FROM (" . $query . ") AS result
	")
      ->fetchField();

  // Set the query
  $result = db_query(
      $query . "
		LIMIT
			" . $limit_from . ", " . $limit_to . "
	");

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      $data->agent_fullname,
      $data->domain_name,
      $data->chats,
    );
  }

  pager_default_initialize($limit_max, $activehelper_livehelp['listing_limit']);

  // Render the table
  $output .= theme_table(array(
        'header' => $table_header,
        'rows' => $table_rows,
        'attributes' => array('width' => '100%'),
        'sticky' => TRUE,
        'caption' => '',
        'colgroups' => array(),
        'empty' => t('No chats found from @from to @to.', array('@from' => $date_from, '@to' => $date_to))
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__monthly_chats_date_form($form, &$form_state) {
  if (!isset($_GET['from']) || !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['from'])) {
    $_GET['from'] = (string) date('Y-m-d');
  }
  if (!isset($_GET['to']) || !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['to'])) {
    $_GET['to'] = (string) date('Y-m-d');
  }

  $date_from = explode('-', $_GET['from']);
  $date_to = explode('-', $_GET['to']);

  $form['controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['controls']['from'] = array(
    '#type' => 'date',
    '#title' => t('From'),
    '#default_value' => array(
      'year' => intval($date_from[0], 10),
      'month' => intval($date_from[1], 10),
      'day' => intval($date_from[2], 10)
    )
  );
  $form['controls']['to'] = array(
    '#type' => 'date',
    '#title' => t('To'),
    '#default_value' => array(
      'year' => intval($date_to[0], 10),
      'month' => intval($date_to[1], 10),
      'day' => intval($date_to[2], 10)
    )
  );

  $form['controls']['submit_1'] = array(
    '#name' => 'op__apply',
    '#type' => 'submit',
    '#value' => t('Apply'),
  );
  $form['controls']['submit_2'] = array(
    '#name' => 'op__export',
    '#type' => 'submit',
    '#value' => t('Export all to CSV'),
  );

  return $form;
}

function activehelper_livehelp__monthly_chats_date_form_submit($form, &$form_state) {
  $date_from = date('Y-m-d');
  $date_to = $date_from;

  $from = &$form_state['values']['from'];
  if (checkdate($from['month'], $from['day'], $from['year'])) {
    $from['month'] = ( $from['month'] < 10 ) ? '0' . $from['month'] : $from['month'];
    $from['day'] = ( $from['day'] < 10 ) ? '0' . $from['day'] : $from['day'];
    $date_from = $from['year'] . '-' . $from['month'] . '-' . $from['day'];
  }

  $to = &$form_state['values']['to'];
  if (checkdate($to['month'], $to['day'], $to['year'])) {
    $to['month'] = ( $to['month'] < 10 ) ? '0' . $to['month'] : $to['month'];
    $to['day'] = ( $to['day'] < 10 ) ? '0' . $to['day'] : $to['day'];
    $date_to = $to['year'] . '-' . $to['month'] . '-' . $to['day'];
  }

  $_GET['from'] = $date_from;
  $_GET['to'] = $date_to;

  if (isset($form_state['values']['op__export'])) {
    activehelper_livehelp__monthly_chats_export_csv();
  }

  $query = array(
    'order' => $_GET['order'],
    'sort' => $_GET['sort'],
    'page' => $_GET['page'],
    'from' => $_GET['from'],
    'to' => $_GET['to']
  );
  drupal_goto('admin/activehelper-livehelp/chats', array('query' => $query));

  return $form;
}

function activehelper_livehelp__monthly_chats_export_csv() {
  switch ($_GET['order']) {
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Chats'):
      $order_field = "COUNT( jls.id )";
      break;
    case t('Agent'):
    default:
      $order_field = "CONCAT( jlu.firstname, ' ', jlu.lastname )";
  }
  $order_type = $_GET['sort'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare CSV header
  $output = '"' . t('Agent') . '","' . t('Domain name') . '","' . t('Chats') . '"';

  $query = "
		SELECT
			CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_fullname,
			jld.name AS domain_name,
			COUNT( jls.id ) AS chats
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jls.datetime, '%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jls.datetime, '%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jls.id_user = jlu.id
			AND jls.id_domain = jld.id_domain
		GROUP BY
			jls.id_domain, jlu.username, CONCAT( jlu.firstname, ' ', jlu.lastname )
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  // Set the query
  $result = db_query($query);

  $rows = '';
  while ($data = $result->fetchObject()) {
    $rows .= "\n" . '"' . $data->agent_fullname . '","' . $data->domain_name . '","' . $data->chats . '"';
  }

  if ($rows == '') {
    $output .= "\n" . t('No chats found from @from to @to.', array('@from' => $date_from, '@to' => $date_to));
  }
  else {
    $output .= $rows;
  }

  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=LiveHelp_MonthlyChats_" . $date_from . "~" . $date_to . ".csv");
  header("Content-Type: text/csv; text/comma-separated-values");
  header("Content-Transfer-Encoding: binary");

  echo $output;

  // Finally: we should not let Drupal print anything else.
  die;
}

