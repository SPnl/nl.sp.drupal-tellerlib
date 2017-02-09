<?php

function tellerlib_detail($config) {
  // initialize civicrm
  civicrm_initialize();

  $html = '<main class="content">';

  // get query string parameters
  $queryString = drupal_get_query_parameters();

  // check if we have filters in the query string
  $filterProvincie = '';
  if (array_key_exists('filterProvincie', $queryString)) {
    $filterProvincie = urldecode($queryString['filterProvincie']);
  }

  $filterGemeente = '';
  if (array_key_exists('filterGemeente', $queryString)) {
    $filterGemeente = urldecode($queryString['filterGemeente']);
  }
  
  $filterWoonplaats = '';
  if (array_key_exists('filterWoonplaats', $queryString)) {
    $filterWoonplaats = urldecode($queryString['filterWoonplaats']);
  }

  // get the data
  $data = tellerlib_detail_getData($config, $filterProvincie, $filterGemeente, $filterWoonplaats);

  // set the drill up link and the page title
  $html .= '<p>' . $data['drillUpURL'] . '</a></p>';
  $html .= '<h2>' . $data['pageTitle'] . '</h2>';

  // show the data in a table
  $html .= '<table class="tellerdetail">';
  $html .= '<tr>';
  $html .= "<th>{$data['columnHeader']}</th>";
  $html .= "<th>Aantal</th>";
  $html .= '</tr>';

  while ($data['dao']->fetch()) {
    $html .= '<tr>';

    if ($data['drillDownURL']) {
      $html .= '<td class="telleritem"><a href="' . $data['drillDownURL'] . urlencode($data['dao']->item) . '">' . $data['dao']->item . '</a></td>';
    }
    else {
      // no drill down on item
      $html .= "<td class=\"telleritem\">{$data['dao']->item}</td>";
    }

    $html .= "<td class=\"tellervalue\">{$data['dao']->aantal}</td>";
    $html .= '</tr>';
  }

  $html .= '</table>';

  $html .= '</main>';

  return $html;
}

function tellerlib_detail_getData($config, $filterProvincie, $filterGemeente, $filterWoonplaats) {
  // prepare return value array
  $retval = array(
    'pageTitle' => '',
    'drillUpURL' => '',
    'drillUpDownURL' => '',
    'columnHeader' => '',
    'dao' => '',
  );

  $sqlParams = array(
    1 => array($config->groupID, 'Integer'),
  );

  if ($filterProvincie == '') {
    $retval['drillUpURL'] = "<a href=\"{$config->overviewPageURL}\">&lt terug naar overzicht</a>";
    $retval['pageTitle'] = 'Alle provincies';
    $retval['columnHeader'] = 'Provincie';
    $retval['drillDownURL'] = "{$config->detailPageURL}?filterProvincie=";

    $sql = "
        SELECT
          ifnull(ag.`provincie_28`, '(niet ingevuld)') item
          , COUNT(gc.id) aantal
        FROM         
      	  civicrm_value_adresgegevens_12 ag
        LEFT OUTER JOIN
          civicrm_address a ON a.id = ag.`entity_id`
        LEFT OUTER JOIN
          civicrm_contact c ON c.id = a.`contact_id`
        LEFT OUTER JOIN
          civicrm_group_contact gc ON gc.`contact_id` = c.id	   
        WHERE
          gc.group_id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND a.`is_primary` = 1
        GROUP BY
          ag.`provincie_28`
        ORDER BY 1
      ";
  }
  else if ($filterGemeente == '') {
    $retval['drillUpURL'] = "<a href=\"{$config->detailPageURL}\">&lt terug naar alle provincies</a>";
    $retval['pageTitle'] = "Provincie $filterProvincie";
    $retval['columnHeader'] = 'Gemeente';
    $retval['drillDownURL'] = "{$config->detailPageURL}?filterProvincie=" . urlencode($filterProvincie) . '&filterGemeente=';

    $sql = "
        SELECT
          ifnull(ag.`gemeente_24`, '(niet ingevuld)') item
          , COUNT(gc.id) aantal
        FROM         
      	  civicrm_value_adresgegevens_12 ag
        LEFT OUTER JOIN
          civicrm_address a ON a.id = ag.`entity_id`
        LEFT OUTER JOIN
          civicrm_contact c ON c.id = a.`contact_id`
        LEFT OUTER JOIN
          civicrm_group_contact gc ON gc.`contact_id` = c.id	   
        WHERE
          gc.group_id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND a.`is_primary` = 1
          AND ag.`provincie_28` = %2
        GROUP BY
          ag.`gemeente_24`
        ORDER BY 1
      ";

    $sqlParams[2] = array($filterProvincie, 'String');
  }
  else if ($filterWoonplaats == '') {
    $retval['drillUpURL'] = "<a href=\"{$config->detailPageURL}?filterProvincie=" . urlencode($filterProvincie) . '">&lt terug naar provincie ' . $filterProvincie . '</a>';
    $retval['pageTitle'] = "Gemeente $filterGemeente ($filterProvincie)";
    $retval['columnHeader'] = 'Woonplaats';
    $retval['drillDownURL'] = "{$config->detailPageURL}?filterProvincie=" . urlencode($filterProvincie) . '&filterGemeente=' . urlencode($filterGemeente) . '&filterWoonplaats=';

    $sql = "
        SELECT
          ifnull(a.city, '(niet ingevuld)') item
          , COUNT(gc.id) aantal
        FROM         
      	  civicrm_value_adresgegevens_12 ag
        LEFT OUTER JOIN
          civicrm_address a ON a.id = ag.`entity_id`
        LEFT OUTER JOIN
          civicrm_contact c ON c.id = a.`contact_id`
        LEFT OUTER JOIN
          civicrm_group_contact gc ON gc.`contact_id` = c.id	   
        WHERE
          gc.group_id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND a.`is_primary` = 1
          AND ag.`provincie_28` = %2
          AND ag.`gemeente_24` = %3 
        GROUP BY
          a.city
        ORDER BY 1
      ";

    $sqlParams[2] = array($filterProvincie, 'String');
    $sqlParams[3] = array($filterGemeente, 'String');
  }
  else {
    $retval['drillUpURL'] = "<a href=\"{$config->detailPageURL}?filterProvincie=" . urlencode($filterProvincie) .
      '&filterGemeente=' . urlencode($filterGemeente) . '">&lt terug naar gemeente ' . $filterGemeente . '</a>';
    $retval['pageTitle'] = "Woonplaats $filterWoonplaats (gemeente $filterGemeente)";
    $retval['columnHeader'] = 'Buurt';
    $retval['drillDownURL'] = '';

    $sql = "
        SELECT
          ifnull(ag.buurt_25, '(niet ingevuld)') item
          , COUNT(gc.id) aantal
        FROM         
      	  civicrm_value_adresgegevens_12 ag
        LEFT OUTER JOIN
          civicrm_address a ON a.id = ag.`entity_id`
        LEFT OUTER JOIN
          civicrm_contact c ON c.id = a.`contact_id`
        LEFT OUTER JOIN
          civicrm_group_contact gc ON gc.`contact_id` = c.id	   
        WHERE
          gc.group_id = %1
          AND c.is_deleted = 0
          AND gc.status = 'Added'
          AND a.`is_primary` = 1
          AND ag.`provincie_28` = %2
          AND ag.`gemeente_24` = %3
          AND a.city = %4
        GROUP BY
          ag.buurt_25
        ORDER BY 1
      ";

    $sqlParams[2] = array($filterProvincie, 'String');
    $sqlParams[3] = array($filterGemeente, 'String');
    $sqlParams[4] = array($filterWoonplaats, 'String');
  }

  $retval['dao'] = CRM_Core_DAO::executeQuery($sql, $sqlParams);

  return $retval;
}

