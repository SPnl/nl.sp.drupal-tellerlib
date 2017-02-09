<?php

function tellerlib_overzicht($config) {
  // initialize civicrm
  civicrm_initialize();

  $html = '<main class="content">';
  $html .= "<p>{$config->overviewPageTitle}</p>";

  // 1. count the total number of supporters
  $sql = "
    SELECT
      COUNT(gc.id)
    FROM
      civicrm_group g
    INNER JOIN
      civicrm_group_contact gc ON gc.`group_id` = g.id
    INNER JOIN
      civicrm_contact c ON gc.contact_id = c.id
    WHERE
      g.id = %1
      and c.is_deleted = 0
      and gc.status = 'Added'
  ";
  $sqlParams = array(
    1 => array($config->groupID, 'Integer'),
  );
  $totalNumber = CRM_Core_DAO::singleValueQuery($sql, $sqlParams);

  $html .= '<h2>Landelijk</h2>';
  $html .= "<ul><li><a href=\"{$config->detailPageURL}\">Totaal</a>: $totalNumber</li></ul>";

  // 2. count the number of supporters per local branch
  $sqlExtraWhere = tellerlib_limitBranchList();
  $sql = "
    SELECT
      afd.`display_name` as afdeling
      , COUNT(gc.id) as aantal
    FROM
      civicrm_group g
    INNER JOIN
      civicrm_group_contact gc ON gc.`group_id` = g.id
    INNER JOIN
      civicrm_contact c ON gc.contact_id = c.id
    INNER JOIN
      civicrm_value_geostelsel gs ON gs.`entity_id` = gc.`contact_id`
    INNER JOIN
      civicrm_contact afd ON gs.`afdeling` = afd.`id`
    WHERE
      g.id = %1
      and c.is_deleted = 0
      and gc.status = 'Added'
      AND $sqlExtraWhere
    GROUP BY
      afd.`display_name`
    ORDER BY
      afd.`sort_name`
  ";
  $sqlParams = array(
    1 => array($config->groupID, 'Integer'),
  );

  $html .= '<h2>Afdeling</h2>';
  $html .= '<ul>';

  $dao = CRM_Core_DAO::executeQuery($sql, $sqlParams);
  while($dao->fetch()) {
    $html .= "<li>{$dao->afdeling}: {$dao->aantal}</li>";
  }

  $html .= '</ul>';
  $html .= '</main>';

  return $html;
}

function tellerlib_limitBranchList() {
  global $user;

  // check if the user has full or limited permissions
  $userHasFullPermissions = FALSE;
  foreach ($user->roles as $role) {
    if ($role == 'administrator' || $role == 'Applicatiebeheer CiviCRM' || $role == 'Personeel') {
      $userHasFullPermissions = TRUE;
      break;
    }
  }

  if ($userHasFullPermissions) {
    // this user can see all branches, add a dummy where clause
    $sqlExtraWhere = ' 1 = 1 ';
  }
  else {
    // get the contact id of the current user
    $params = array(
      'uf_id' => $user->uid,
      'sequential' => 1,
    );
    $results = civicrm_api3('UFMatch', 'get', $params);

    if ($results['count'] == 1) {
      // get the branch of the current user
      $sql = "select afdeling from civicrm_value_geostelsel where entity_id = %1";
      $sqlParams = array(
        1 => array($results['values'][0]['contact_id'], 'Integer'),
      );
      $branch = CRM_Core_DAO::singleValueQuery($sql, $sqlParams);

      if (!empty($branch)) {
        // limit the query to the branch of this user
        $sqlExtraWhere = ' gs.afdeling = ' . $branch;
      }
      else {
        // user has no branch, make sure the query will return no records
        $sqlExtraWhere = ' 1 = 0 ';
      }
    }
    else {
      // user has no corresponding contact in CiviCRM, make sure the query will return no records
      $sqlExtraWhere = ' 1 = 0 ';
    }
  }

  return $sqlExtraWhere;
}
