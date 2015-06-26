<?php

function activehelper_livehelp__offline_messages() {
  global $activehelper_livehelp;

  $output = '<h2>Offline messages</h2>';

  drupal_add_css('div.form-item-from { float: left; margin-right: 25px; } .table-cell-nowrap { white-space: nowrap; }', 'inline');

  // Define the default values for the table sort
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Answered');
  }
  if (!isset($_GET['sort'])) {
    $_GET['sort'] = 'asc';
  }
  if (!isset($_GET['page'])) {
    $_GET['page'] = 0;
  }

  $form = drupal_get_form('activehelper_livehelp__offline_messages_date_form');
  $output .= render($form);

  if (isset($_GET['message_answered'])) {
    activehelper_livehelp__offline_messages_change_answered($_GET['message_answered']);
  }
  if (isset($_GET['message_delete'])) {
    activehelper_livehelp__offline_messages_delete($_GET['message_delete']);
  }

  switch ($_GET['order']) {
    case t('Visitor name'):
      $order_field = "jlom.name";
      break;
    case t('Visitor email'):
      $order_field = "jlom.email";
      break;
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Date'):
      $order_field = "jlom.datetime";
      break;
    case t('Message'):
      $order_field = "COUNT( jls.id )";
      break;
    case t('Answered'):
    default:
      $order_field = "jlom.answered";
  }
  $order_type = $_GET['sort'];

  $limit_from = $_GET['page'] * $activehelper_livehelp['listing_limit'];
  $limit_to = ( $_GET['page'] + 1 ) * $activehelper_livehelp['listing_limit'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare table header
  $table_header = array(
    array('data' => t('Status'), 'field' => 'answered'),
    array('data' => t('Visitor name'), 'field' => 'visitor_name', 'class' => array('table-cell-nowrap')),
    array('data' => t('Visitor email'), 'field' => 'visitor_email', 'class' => array('table-cell-nowrap')),
    array('data' => t('Domain name'), 'field' => 'domain_name', 'class' => array('table-cell-nowrap')),
    array('data' => t('Date'), 'field' => 'date', 'class' => array('table-cell-nowrap')),
    array('data' => t('Message')),
    array('data' => t('Operations'), 'class' => array('table-cell-nowrap')),
  );

  $query = "
		SELECT
			jlom.id AS message_id, jlom.name AS visitor_name, jlom.email AS visitor_email,
			jld.name AS domain_name, jlom.datetime AS date, jlom.message, jlom.answered
		FROM
			{livehelp_offline_messages} AS jlom,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jlom.datetime,'%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jlom.datetime, '%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jlom.id_domain = jld.id_domain
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  $limit_max = db_query("
		SELECT COUNT( result.message_id )
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
      l(
          '<img src="' . $activehelper_livehelp['images_url'] . '/' . $data->answered . '.gif" alt="" />', 'admin/activehelper-livehelp/offline-messages', array(
        'html' => TRUE,
        'query' => array(
          'message_answered' => $data->message_id,
          'order' => $_GET['order'],
          'sort' => $_GET['sort'],
          'page' => $_GET['page'],
          'from' => $_GET['from'],
          'to' => $_GET['to']
        )
          )
      ),
      $data->visitor_name,
      $data->visitor_email,
      $data->domain_name,
      array('data' => $data->date, 'class' => array('table-cell-nowrap')),
      $data->message,
      array(
        'data' =>
        l(
            $data->answered ? t('Mark as not answered') : t('Mark as answered'), 'admin/activehelper-livehelp/offline-messages', array(
          'query' => array(
            'message_answered' => $data->message_id,
            'order' => $_GET['order'],
            'sort' => $_GET['sort'],
            'page' => $_GET['page'],
            'from' => $_GET['from'],
            'to' => $_GET['to']
          )
            )
        ) . ' &nbsp;|&nbsp; ' .
        l(
            t('Delete'), 'admin/activehelper-livehelp/offline-messages', array(
          'attributes' => array(
            'onclick' => "return window.confirm( '" . t('Are you sure you want to delete this offline message?') . "' );"
          ),
          'query' => array(
            'message_delete' => $data->message_id,
            'order' => $_GET['order'],
            'sort' => $_GET['sort'],
            'page' => $_GET['page'],
            'from' => $_GET['from'],
            'to' => $_GET['to']
          )
            )
        ),
        'class' => array('table-cell-nowrap')
      )
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
        'empty' => t('No offline messages found from @from to @to.', array('@from' => $date_from, '@to' => $date_to))
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__offline_messages_date_form($form, &$form_state) {
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

function activehelper_livehelp__offline_messages_date_form_submit($form, &$form_state) {
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
    activehelper_livehelp__offline_messages_export_csv();
  }

  $query = array(
    'order' => $_GET['order'],
    'sort' => $_GET['sort'],
    'page' => $_GET['page'],
    'from' => $_GET['from'],
    'to' => $_GET['to']
  );
  drupal_goto('admin/activehelper-livehelp/offline-messages', array('query' => $query));

  return $form;
}

function activehelper_livehelp__offline_messages_delete($message_id = 0) {
  $message_id = (int) $message_id;

  db_query("
		DELETE FROM {livehelp_offline_messages}
		WHERE id = '{$message_id}'
		LIMIT 1
	");

  drupal_set_message(t('The offline message has been removed.'));
}

function activehelper_livehelp__offline_messages_change_answered($message_id = 0) {
  $message_id = (int) $message_id;

  db_query("
		UPDATE {livehelp_offline_messages}
		SET answered = IF(answered <> 1, 1, 0)
		WHERE id = '{$message_id}'
	");

  drupal_set_message(t('The offline message has been updated.'));
}

function activehelper_livehelp__offline_messages_export_csv() {
  switch ($_GET['order']) {
    case t('Visitor name'):
      $order_field = "jlom.name";
      break;
    case t('Visitor email'):
      $order_field = "jlom.email";
      break;
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Date'):
      $order_field = "jlom.datetime";
      break;
    case t('Message'):
      $order_field = "COUNT( jls.id )";
      break;
    case t('Answered'):
    default:
      $order_field = "jlom.answered";
  }
  $order_type = $_GET['sort'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare CSV header
  $output = '"' . t('Answered') . '","' . t('Visitor name') . '","' . t('Visitor email') . '","'
      . t('Domain name') . '","' . t('Date') . '","' . t('Message') . '"';

  $query = "
		SELECT
			jlom.id AS message_id, jlom.name AS visitor_name, jlom.email AS visitor_email,
			jld.name AS domain_name, jlom.datetime AS date, jlom.message, jlom.answered
		FROM
			{livehelp_offline_messages} AS jlom,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jlom.datetime,'%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jlom.datetime, '%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jlom.id_domain = jld.id_domain
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  // Set the query
  $result = db_query($query);

  $rows = '';
  while ($data = $result->fetchObject()) {
    $rows .= "\n" . '"' . ( $data->answered ? t('Yes') : t('No') ) . '","' . $data->visitor_name . '","'
        . $data->visitor_email . '","' . $data->domain_name . '","' . $data->date . '","' . $data->message . '"';
  }

  if ($rows == '') {
    $output .= "\n" . t('No offline messages found from @from to @to.', array('@from' => $date_from, '@to' => $date_to));
  }
  else {
    $output .= $rows;
  }

  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=LiveHelp_OfflineMessages_" . $date_from . "~" . $date_to . ".csv");
  header("Content-Type: text/csv; text/comma-separated-values");
  header("Content-Transfer-Encoding: binary");

  echo $output;

  // Finally: we should not let Drupal print anything else.
  die;
}

